/**
 * @file step_dir_driver.h
 * @brief GPIO-based step/dir driver implementation
 *
 * This driver provides basic step/dir control using GPIO pins.
 * It is the simplest implementation and serves as a fallback when
 * TMC UART drivers are not available.
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#ifndef USD_STEP_DIR_DRIVER_H
#define USD_STEP_DIR_DRIVER_H

#include "idriver.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
// For native testing, provide stubs
#include <cstdint>
#endif

namespace usd {

/**
 * @brief GPIO-based step/dir driver
 *
 * This driver uses direct GPIO manipulation for step pulse generation.
 * Maximum reliable frequency depends on the task loop rate (typically ~20kHz).
 *
 * For high-speed stepping (>20kHz), use MCPWM or Timer-based stepping.
 *
 * Thread Safety: NOT thread-safe. Call only from MotionTask.
 */
class GenericStepDirDriver : public IDriver {
public:
    /**
     * @brief Construct driver with pin and timing configuration
     *
     * @param pins Pin configuration (step, dir, enable GPIOs)
     * @param timing Step timing configuration
     */
    GenericStepDirDriver(const DriverPins& pins, const StepTiming& timing);

    /**
     * @brief Destructor - ensures driver is disabled
     */
    ~GenericStepDirDriver() override;

    // ─────────────────────────────────────────────────────────────────────
    // IDriver Implementation
    // ─────────────────────────────────────────────────────────────────────

    bool enable() override;
    void disable() override;
    bool isEnabled() const override;
    DriverState getState() const override;

    void setDirection(Direction dir) override;
    Direction getDirection() const override;

    bool step() override;
    uint32_t stepMultiple(uint32_t count, uint32_t steps_per_second) override;

    int32_t getPosition() const override;
    void setPosition(int32_t position) override;

    const DriverPins& getPins() const override;
    const StepTiming& getTiming() const override;

    bool isFault() const override;
    bool clearFault() override;
    uint8_t getFaultCode() const override;

    // ─────────────────────────────────────────────────────────────────────
    // Additional Methods
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Initialize GPIO pins
     *
     * Called automatically by constructor, but can be called again
     * if pins need to be reinitialized.
     */
    void initPins();

private:
    DriverPins pins_;
    StepTiming timing_;
    DriverState state_;
    Direction direction_;
    int32_t position_;

    /**
     * @brief Generate a single step pulse with proper timing
     */
    void pulseStep();

    /**
     * @brief Apply enable state to hardware
     * @param enabled True to enable, false to disable
     */
    void setEnablePin(bool enabled);

    /**
     * @brief Apply direction to hardware
     */
    void updateDirectionPin();

    /**
     * @brief Microsecond delay (platform-independent)
     * @param us Microseconds to delay
     */
    static void delayMicroseconds(uint32_t us);

    /**
     * @brief Millisecond delay (platform-independent)
     * @param ms Milliseconds to delay
     */
    static void delayMilliseconds(uint32_t ms);
};

}  // namespace usd

#endif  // USD_STEP_DIR_DRIVER_H
