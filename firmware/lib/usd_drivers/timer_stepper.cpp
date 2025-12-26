/**
 * @file timer_stepper.cpp
 * @brief Timer-based step pulse generator implementation
 *
 * Uses esp_timer for ISR-based step pulse generation. This is a fallback
 * for platforms without MCPWM or when simpler timing is preferred.
 *
 * @version 0.1.0
 * @date 2025-05-28
 */

#include "timer_stepper.h"

#ifdef ARDUINO
#include <Arduino.h>
#include "esp_timer.h"
#else
// Native testing stubs - no hardware operations
#include <chrono>
#endif

namespace usd {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

TimerStepper::TimerStepper(int8_t step_pin, const TimerConfig& config, uint16_t pulse_width_us)
    : step_pin_(step_pin)
    , config_(config)
    , pulse_width_us_(pulse_width_us)
    , state_(StepGenState::IDLE)
    , frequency_hz_(0)
    , step_count_(0)
    , target_steps_(0)
    , initialized_(false)
    , pulse_high_(false)
#ifdef ARDUINO
    , step_timer_(nullptr)
    , pulse_end_timer_(nullptr)
#endif
{
}

TimerStepper::~TimerStepper()
{
    stop();
    
#ifdef ARDUINO
    // Delete timers if they exist
    if (step_timer_ != nullptr) {
        esp_timer_delete(step_timer_);
        step_timer_ = nullptr;
    }
    if (pulse_end_timer_ != nullptr) {
        esp_timer_delete(pulse_end_timer_);
        pulse_end_timer_ = nullptr;
    }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

bool TimerStepper::init()
{
#ifdef ARDUINO
    // Validate configuration
    if (step_pin_ < 0) {
        return false;
    }

    // Configure GPIO as output
    pinMode(step_pin_, OUTPUT);
    digitalWrite(step_pin_, LOW);

    // Create the step timer (periodic)
    if (step_timer_ == nullptr) {
        esp_timer_create_args_t step_timer_args = {
            .callback = &TimerStepper::stepTimerCallback,
            .arg = this,
            .dispatch_method = ESP_TIMER_ISR,  // Run in ISR context for low latency
            .name = config_.timer_name,
            .skip_unhandled_events = config_.skip_unhandled
        };
        
        esp_err_t err = esp_timer_create(&step_timer_args, &step_timer_);
        if (err != ESP_OK) {
            return false;
        }
    }

    // Create the pulse-end timer (one-shot, for controlling pulse width)
    if (pulse_end_timer_ == nullptr) {
        esp_timer_create_args_t pulse_end_args = {
            .callback = &TimerStepper::pulseEndCallback,
            .arg = this,
            .dispatch_method = ESP_TIMER_ISR,
            .name = "pulse_end",
            .skip_unhandled_events = true
        };
        
        esp_err_t err = esp_timer_create(&pulse_end_args, &pulse_end_timer_);
        if (err != ESP_OK) {
            // Clean up step timer
            esp_timer_delete(step_timer_);
            step_timer_ = nullptr;
            return false;
        }
    }

    initialized_ = true;
#else
    // Native build: always succeed for testing
    initialized_ = true;
#endif

    return initialized_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Control
// ─────────────────────────────────────────────────────────────────────────────

bool TimerStepper::start()
{
    if (!initialized_ || frequency_hz_ == 0) {
        return false;
    }

#ifdef ARDUINO
    // Calculate period in microseconds
    uint64_t period_us = calculatePeriodUs(frequency_hz_);
    
    // Start the periodic timer
    esp_err_t err = esp_timer_start_periodic(step_timer_, period_us);
    if (err != ESP_OK) {
        return false;
    }
#endif

    state_ = StepGenState::RUNNING;
    pulse_high_ = false;
    
    return true;
}

void TimerStepper::stop()
{
    if (state_ == StepGenState::IDLE) {
        return;
    }

#ifdef ARDUINO
    // Stop both timers
    if (step_timer_ != nullptr) {
        esp_timer_stop(step_timer_);
    }
    if (pulse_end_timer_ != nullptr) {
        esp_timer_stop(pulse_end_timer_);
    }

    // Ensure output is low
    digitalWrite(step_pin_, LOW);
#endif

    pulse_high_ = false;
    state_ = StepGenState::IDLE;
}

bool TimerStepper::isRunning() const
{
    return state_ == StepGenState::RUNNING;
}

StepGenState TimerStepper::getState() const
{
    return state_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Frequency Control
// ─────────────────────────────────────────────────────────────────────────────

bool TimerStepper::setFrequency(uint32_t frequency_hz)
{
    if (frequency_hz < MIN_FREQUENCY || frequency_hz > MAX_FREQUENCY) {
        return false;
    }

    frequency_hz_ = frequency_hz;

#ifdef ARDUINO
    if (state_ == StepGenState::RUNNING) {
        // Update timer period on the fly
        updateTimerPeriod();
    }
#endif

    return true;
}

uint32_t TimerStepper::getFrequency() const
{
    return frequency_hz_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Step Counting
// ─────────────────────────────────────────────────────────────────────────────

uint32_t TimerStepper::getStepCount() const
{
    return step_count_;
}

void TimerStepper::resetStepCount()
{
    step_count_ = 0;
}

void TimerStepper::setTargetSteps(uint32_t target_steps)
{
    target_steps_ = target_steps;
}

// ─────────────────────────────────────────────────────────────────────────────
// Timer Callbacks (ISR Context)
// ─────────────────────────────────────────────────────────────────────────────

#ifdef ARDUINO

void IRAM_ATTR TimerStepper::stepTimerCallback(void* arg)
{
    TimerStepper* self = static_cast<TimerStepper*>(arg);
    
    // Check if we should auto-stop
    if (self->target_steps_ > 0 && self->step_count_ >= self->target_steps_) {
        // Stop the timer from ISR context
        esp_timer_stop(self->step_timer_);
        digitalWrite(self->step_pin_, LOW);
        self->state_ = StepGenState::IDLE;
        return;
    }
    
    // Generate step pulse - rising edge
    digitalWrite(self->step_pin_, HIGH);
    self->step_count_++;
    self->pulse_high_ = true;
    
    // Schedule the falling edge after pulse_width_us
    esp_timer_start_once(self->pulse_end_timer_, self->pulse_width_us_);
}

void IRAM_ATTR TimerStepper::pulseEndCallback(void* arg)
{
    TimerStepper* self = static_cast<TimerStepper*>(arg);
    
    // End the step pulse - falling edge
    digitalWrite(self->step_pin_, LOW);
    self->pulse_high_ = false;
}

#endif

// ─────────────────────────────────────────────────────────────────────────────
// Private Methods
// ─────────────────────────────────────────────────────────────────────────────

uint64_t TimerStepper::calculatePeriodUs(uint32_t freq_hz) const
{
    if (freq_hz == 0) {
        return 0;
    }
    
    // Period = 1 / frequency, converted to microseconds
    // Use 64-bit to avoid overflow
    return 1000000ULL / static_cast<uint64_t>(freq_hz);
}

void TimerStepper::updateTimerPeriod()
{
#ifdef ARDUINO
    if (step_timer_ == nullptr || frequency_hz_ == 0) {
        return;
    }
    
    // Stop current timer
    esp_timer_stop(step_timer_);
    
    // Restart with new period
    uint64_t period_us = calculatePeriodUs(frequency_hz_);
    esp_timer_start_periodic(step_timer_, period_us);
#endif
}

}  // namespace usd
