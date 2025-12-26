/**
 * @file idriver.h
 * @brief Abstract driver interface for stepper motor drivers
 *
 * This interface defines the contract for all motor driver implementations.
 * Concrete implementations include GenericStepDirDriver (GPIO-based),
 * TMC2208Driver (UART-configured), and TMC2209Driver (with StallGuard).
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#ifndef USD_IDRIVER_H
#define USD_IDRIVER_H

#include <cstdint>

namespace usd {

/**
 * @brief Motor rotation direction
 */
enum class Direction : uint8_t {
    CW = 0,   ///< Clockwise (positive)
    CCW = 1   ///< Counter-clockwise (negative)
};

/**
 * @brief Driver operational state
 */
enum class DriverState : uint8_t {
    DISABLED = 0,    ///< Driver outputs disabled (motor free)
    ENABLED = 1,     ///< Driver enabled, ready to step
    FAULT = 2        ///< Driver in fault condition
};

/**
 * @brief Configuration for step/dir/enable pins
 */
struct DriverPins {
    int8_t step_pin;      ///< GPIO for step pulse (-1 = not used)
    int8_t dir_pin;       ///< GPIO for direction (-1 = not used)
    int8_t enable_pin;    ///< GPIO for enable (-1 = not used)
    bool enable_active_low;  ///< True if enable is active-low (typical)
    bool dir_invert;         ///< True to invert direction logic
};

/**
 * @brief Step timing configuration
 */
struct StepTiming {
    uint16_t step_pulse_us;    ///< Step pulse width in microseconds (typ. 2-5)
    uint16_t dir_setup_us;     ///< Direction setup time before step (typ. 5-20)
    uint16_t enable_delay_ms;  ///< Delay after enable before stepping (typ. 5-10)
};

/**
 * @brief Abstract interface for stepper motor drivers
 *
 * All driver implementations must inherit from this interface.
 * This enables the MotionController to work with any driver type
 * (GPIO, TMC UART, etc.) through polymorphism.
 *
 * Usage:
 * @code
 *   IDriver* driver = new GenericStepDirDriver(pins, timing);
 *   driver->enable();
 *   driver->setDirection(Direction::CW);
 *   driver->step();  // Generate single step pulse
 * @endcode
 */
class IDriver {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~IDriver() = default;

    // ─────────────────────────────────────────────────────────────────────
    // Enable/Disable Control
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Enable the driver outputs
     *
     * After enabling, there may be a delay before stepping is allowed
     * (see StepTiming::enable_delay_ms). The driver should handle this
     * internally.
     *
     * @return true if enable succeeded, false if driver is in fault
     */
    virtual bool enable() = 0;

    /**
     * @brief Disable the driver outputs
     *
     * Motor will be free to rotate. This is the safe state.
     */
    virtual void disable() = 0;

    /**
     * @brief Check if driver is enabled
     * @return true if enabled and ready to step
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief Get current driver state
     * @return DriverState enum value
     */
    virtual DriverState getState() const = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Direction Control
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Set motor rotation direction
     *
     * Direction change takes effect on next step pulse.
     * Respects dir_setup_us timing before step is issued.
     *
     * @param dir Direction::CW or Direction::CCW
     */
    virtual void setDirection(Direction dir) = 0;

    /**
     * @brief Get current direction setting
     * @return Current direction
     */
    virtual Direction getDirection() const = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Step Generation
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Generate a single step pulse
     *
     * This is a blocking call that generates one step pulse.
     * For high-speed stepping, use the step generator (MCPWM or Timer ISR).
     *
     * Prerequisites:
     * - Driver must be enabled
     * - Direction must be set (if changed)
     *
     * @return true if step was generated, false if driver not enabled
     */
    virtual bool step() = 0;

    /**
     * @brief Generate multiple step pulses (blocking)
     *
     * Generates the specified number of steps at the given rate.
     * This is a convenience method for simple moves.
     *
     * @param count Number of steps to generate
     * @param steps_per_second Step rate in Hz
     * @return Number of steps actually generated (may be less if interrupted)
     */
    virtual uint32_t stepMultiple(uint32_t count, uint32_t steps_per_second) = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Position Tracking
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get current step position
     *
     * This is a software counter incremented/decremented with each step.
     * Not to be confused with encoder position (which is external).
     *
     * @return Signed step count from zero position
     */
    virtual int32_t getPosition() const = 0;

    /**
     * @brief Set the current position value
     *
     * Used for homing or position synchronization.
     *
     * @param position New position value (typically 0 for homing)
     */
    virtual void setPosition(int32_t position) = 0;

    /**
     * @brief Reset position to zero
     *
     * Convenience method equivalent to setPosition(0)
     */
    virtual void resetPosition() { setPosition(0); }

    // ─────────────────────────────────────────────────────────────────────
    // Configuration
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get driver pin configuration
     * @return Const reference to pin configuration
     */
    virtual const DriverPins& getPins() const = 0;

    /**
     * @brief Get step timing configuration
     * @return Const reference to timing configuration
     */
    virtual const StepTiming& getTiming() const = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Fault Handling
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Check if driver is in fault state
     * @return true if in fault condition
     */
    virtual bool isFault() const = 0;

    /**
     * @brief Attempt to clear fault condition
     *
     * May not succeed if fault condition persists (e.g., overcurrent).
     *
     * @return true if fault cleared, false if fault persists
     */
    virtual bool clearFault() = 0;

    /**
     * @brief Get fault code for diagnostics
     *
     * Fault codes are driver-specific. Generic drivers may only have
     * basic codes, while TMC drivers have detailed diagnostics.
     *
     * @return Driver-specific fault code (0 = no fault)
     */
    virtual uint8_t getFaultCode() const = 0;
};

}  // namespace usd

#endif  // USD_IDRIVER_H
