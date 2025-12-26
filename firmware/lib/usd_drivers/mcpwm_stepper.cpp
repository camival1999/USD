/**
 * @file mcpwm_stepper.cpp
 * @brief MCPWM-based step pulse generator implementation
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#include "mcpwm_stepper.h"

#ifdef ARDUINO
#include <Arduino.h>
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#else
// Native testing stubs - no hardware operations
#include <chrono>
#endif

namespace usd {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

McpwmStepper::McpwmStepper(int8_t step_pin, const McpwmConfig& config, uint16_t pulse_width_us)
    : step_pin_(step_pin)
    , config_(config)
    , pulse_width_us_(pulse_width_us)
    , state_(StepGenState::IDLE)
    , frequency_hz_(0)
    , step_count_(0)
    , target_steps_(0)
    , initialized_(false)
{
}

McpwmStepper::~McpwmStepper()
{
    stop();
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

bool McpwmStepper::init()
{
#ifdef ARDUINO
    // Validate configuration
    if (step_pin_ < 0 || config_.unit > 1 || config_.timer > 2) {
        return false;
    }

    // Get the MCPWM unit
    mcpwm_unit_t unit = static_cast<mcpwm_unit_t>(config_.unit);
    mcpwm_timer_t timer = static_cast<mcpwm_timer_t>(config_.timer);

    // Configure GPIO for MCPWM output
    mcpwm_gpio_init(unit, MCPWM0A, step_pin_);  // MCPWM0A for output A

    // Configure MCPWM timer
    mcpwm_config_t pwm_config = {
        .frequency = 1000,  // Initial frequency (will be changed)
        .cmpr_a = 0,        // Duty cycle for operator A (0% initially)
        .cmpr_b = 0,        // Duty cycle for operator B
        .duty_mode = MCPWM_DUTY_MODE_0,  // Active high
        .counter_mode = MCPWM_UP_COUNTER
    };

    esp_err_t err = mcpwm_init(unit, timer, &pwm_config);
    if (err != ESP_OK) {
        return false;
    }

    // Stop output initially
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);

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

bool McpwmStepper::start()
{
    if (!initialized_ || frequency_hz_ == 0) {
        return false;
    }

#ifdef ARDUINO
    mcpwm_unit_t unit = static_cast<mcpwm_unit_t>(config_.unit);
    mcpwm_timer_t timer = static_cast<mcpwm_timer_t>(config_.timer);

    // Set frequency
    mcpwm_set_frequency(unit, timer, frequency_hz_);

    // Set duty cycle for desired pulse width
    updateDutyCycle();

    // Start the timer
    mcpwm_start(unit, timer);
#endif

    state_ = StepGenState::RUNNING;
    return true;
}

void McpwmStepper::stop()
{
    if (state_ == StepGenState::IDLE) {
        return;
    }

#ifdef ARDUINO
    mcpwm_unit_t unit = static_cast<mcpwm_unit_t>(config_.unit);
    mcpwm_timer_t timer = static_cast<mcpwm_timer_t>(config_.timer);

    // Stop the timer
    mcpwm_stop(unit, timer);

    // Ensure output is low
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
#endif

    state_ = StepGenState::IDLE;
}

bool McpwmStepper::isRunning() const
{
    return state_ == StepGenState::RUNNING;
}

StepGenState McpwmStepper::getState() const
{
    return state_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Frequency Control
// ─────────────────────────────────────────────────────────────────────────────

bool McpwmStepper::setFrequency(uint32_t frequency_hz)
{
    if (frequency_hz < MIN_FREQUENCY || frequency_hz > MAX_FREQUENCY) {
        return false;
    }

    frequency_hz_ = frequency_hz;

#ifdef ARDUINO
    if (state_ == StepGenState::RUNNING) {
        mcpwm_unit_t unit = static_cast<mcpwm_unit_t>(config_.unit);
        mcpwm_timer_t timer = static_cast<mcpwm_timer_t>(config_.timer);

        // Update frequency on the fly
        mcpwm_set_frequency(unit, timer, frequency_hz);

        // Recalculate duty cycle for new frequency
        updateDutyCycle();
    }
#endif

    return true;
}

uint32_t McpwmStepper::getFrequency() const
{
    return frequency_hz_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Step Counting
// ─────────────────────────────────────────────────────────────────────────────

uint32_t McpwmStepper::getStepCount() const
{
    return step_count_;
}

void McpwmStepper::resetStepCount()
{
    step_count_ = 0;
}

void McpwmStepper::setTargetSteps(uint32_t target_steps)
{
    target_steps_ = target_steps;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private Methods
// ─────────────────────────────────────────────────────────────────────────────

void McpwmStepper::updateDutyCycle()
{
#ifdef ARDUINO
    if (frequency_hz_ == 0) {
        return;
    }

    mcpwm_unit_t unit = static_cast<mcpwm_unit_t>(config_.unit);
    mcpwm_timer_t timer = static_cast<mcpwm_timer_t>(config_.timer);

    // Calculate duty cycle to achieve desired pulse width
    float duty = calculateDuty(frequency_hz_);

    // Set the duty cycle
    mcpwm_set_duty(unit, timer, MCPWM_OPR_A, duty);
    mcpwm_set_duty_type(unit, timer, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
#endif
}

float McpwmStepper::calculateDuty(uint32_t freq_hz) const
{
    if (freq_hz == 0) {
        return 0.0f;
    }

    // Period in microseconds
    float period_us = 1000000.0f / static_cast<float>(freq_hz);

    // Duty cycle = pulse_width / period * 100%
    float duty = (static_cast<float>(pulse_width_us_) / period_us) * 100.0f;

    // Clamp to valid range (minimum 0.1% for step detection, max 50%)
    if (duty < 0.1f) {
        duty = 0.1f;
    } else if (duty > 50.0f) {
        // At high frequencies, pulse width becomes significant fraction of period
        // Limit to 50% duty cycle
        duty = 50.0f;
    }

    return duty;
}

}  // namespace usd
