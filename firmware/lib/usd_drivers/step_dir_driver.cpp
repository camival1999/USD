/**
 * @file step_dir_driver.cpp
 * @brief GPIO-based step/dir driver implementation
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#include "step_dir_driver.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
// Native testing stubs - actual GPIO operations are no-ops
#include <chrono>
#include <thread>
#endif

namespace usd {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

GenericStepDirDriver::GenericStepDirDriver(const DriverPins& pins, const StepTiming& timing)
    : pins_(pins)
    , timing_(timing)
    , state_(DriverState::DISABLED)
    , direction_(Direction::CW)
    , position_(0)
{
    initPins();
}

GenericStepDirDriver::~GenericStepDirDriver()
{
    disable();
}

// ─────────────────────────────────────────────────────────────────────────────
// Pin Initialization
// ─────────────────────────────────────────────────────────────────────────────

void GenericStepDirDriver::initPins()
{
#ifdef ARDUINO
    if (pins_.step_pin >= 0) {
        pinMode(pins_.step_pin, OUTPUT);
        digitalWrite(pins_.step_pin, LOW);
    }

    if (pins_.dir_pin >= 0) {
        pinMode(pins_.dir_pin, OUTPUT);
        updateDirectionPin();
    }

    if (pins_.enable_pin >= 0) {
        pinMode(pins_.enable_pin, OUTPUT);
        setEnablePin(false);  // Start disabled
    }
#endif
    // Native build: no GPIO operations
}

// ─────────────────────────────────────────────────────────────────────────────
// Enable/Disable Control
// ─────────────────────────────────────────────────────────────────────────────

bool GenericStepDirDriver::enable()
{
    if (state_ == DriverState::FAULT) {
        return false;  // Cannot enable while in fault
    }

    setEnablePin(true);
    
    // Wait for driver power-up
    if (timing_.enable_delay_ms > 0) {
        delayMilliseconds(timing_.enable_delay_ms);
    }

    state_ = DriverState::ENABLED;
    return true;
}

void GenericStepDirDriver::disable()
{
    setEnablePin(false);
    state_ = DriverState::DISABLED;
}

bool GenericStepDirDriver::isEnabled() const
{
    return state_ == DriverState::ENABLED;
}

DriverState GenericStepDirDriver::getState() const
{
    return state_;
}

void GenericStepDirDriver::setEnablePin(bool enabled)
{
#ifdef ARDUINO
    if (pins_.enable_pin >= 0) {
        // Account for active-low enable (typical for most drivers)
        bool pin_state = pins_.enable_active_low ? !enabled : enabled;
        digitalWrite(pins_.enable_pin, pin_state ? HIGH : LOW);
    }
#endif
    (void)enabled;  // Suppress unused warning in native build
}

// ─────────────────────────────────────────────────────────────────────────────
// Direction Control
// ─────────────────────────────────────────────────────────────────────────────

void GenericStepDirDriver::setDirection(Direction dir)
{
    if (direction_ != dir) {
        direction_ = dir;
        updateDirectionPin();
        
        // Wait for direction setup time
        if (timing_.dir_setup_us > 0) {
            delayMicroseconds(timing_.dir_setup_us);
        }
    }
}

Direction GenericStepDirDriver::getDirection() const
{
    return direction_;
}

void GenericStepDirDriver::updateDirectionPin()
{
#ifdef ARDUINO
    if (pins_.dir_pin >= 0) {
        bool pin_state = (direction_ == Direction::CW);
        if (pins_.dir_invert) {
            pin_state = !pin_state;
        }
        digitalWrite(pins_.dir_pin, pin_state ? HIGH : LOW);
    }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Step Generation
// ─────────────────────────────────────────────────────────────────────────────

bool GenericStepDirDriver::step()
{
    if (state_ != DriverState::ENABLED) {
        return false;
    }

    pulseStep();
    
    // Update position counter
    if (direction_ == Direction::CW) {
        position_++;
    } else {
        position_--;
    }

    return true;
}

uint32_t GenericStepDirDriver::stepMultiple(uint32_t count, uint32_t steps_per_second)
{
    if (state_ != DriverState::ENABLED || count == 0 || steps_per_second == 0) {
        return 0;
    }

    // Calculate delay between steps in microseconds
    uint32_t step_delay_us = 1000000 / steps_per_second;
    
    // Ensure minimum delay accounts for step pulse width
    if (step_delay_us < timing_.step_pulse_us * 2) {
        step_delay_us = timing_.step_pulse_us * 2;
    }

    uint32_t steps_done = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (!step()) {
            break;  // Driver disabled or fault
        }
        steps_done++;
        
        // Delay until next step (subtract pulse time already spent)
        if (i < count - 1) {  // No delay after last step
            delayMicroseconds(step_delay_us - timing_.step_pulse_us);
        }
    }

    return steps_done;
}

void GenericStepDirDriver::pulseStep()
{
#ifdef ARDUINO
    if (pins_.step_pin >= 0) {
        digitalWrite(pins_.step_pin, HIGH);
        delayMicroseconds(timing_.step_pulse_us);
        digitalWrite(pins_.step_pin, LOW);
    }
#else
    // Native build: simulate pulse time
    delayMicroseconds(timing_.step_pulse_us);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Position Tracking
// ─────────────────────────────────────────────────────────────────────────────

int32_t GenericStepDirDriver::getPosition() const
{
    return position_;
}

void GenericStepDirDriver::setPosition(int32_t position)
{
    position_ = position;
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration Getters
// ─────────────────────────────────────────────────────────────────────────────

const DriverPins& GenericStepDirDriver::getPins() const
{
    return pins_;
}

const StepTiming& GenericStepDirDriver::getTiming() const
{
    return timing_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Fault Handling
// ─────────────────────────────────────────────────────────────────────────────

bool GenericStepDirDriver::isFault() const
{
    return state_ == DriverState::FAULT;
}

bool GenericStepDirDriver::clearFault()
{
    if (state_ == DriverState::FAULT) {
        state_ = DriverState::DISABLED;
        return true;
    }
    return false;
}

uint8_t GenericStepDirDriver::getFaultCode() const
{
    // Generic driver has no fault detection hardware
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Platform-Independent Delays
// ─────────────────────────────────────────────────────────────────────────────

void GenericStepDirDriver::delayMicroseconds(uint32_t us)
{
#ifdef ARDUINO
    ::delayMicroseconds(us);
#else
    std::this_thread::sleep_for(std::chrono::microseconds(us));
#endif
}

void GenericStepDirDriver::delayMilliseconds(uint32_t ms)
{
#ifdef ARDUINO
    delay(ms);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

}  // namespace usd
