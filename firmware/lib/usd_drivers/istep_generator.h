/**
 * @file istep_generator.h
 * @brief Abstract interface for step pulse generators
 *
 * This interface defines the contract for all step pulse generator implementations.
 * Concrete implementations include McpwmStepper (MCPWM-based), and future
 * RMT or software-based generators.
 *
 * The step generator is responsible for creating precise step pulses at
 * a specified frequency. It works alongside an IDriver implementation
 * (which handles direction and enable control).
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#ifndef USD_ISTEP_GENERATOR_H
#define USD_ISTEP_GENERATOR_H

#include <cstdint>

namespace usd {

/**
 * @brief Step generator state
 */
enum class StepGenState : uint8_t {
    IDLE = 0,       ///< Not generating steps
    RUNNING = 1,    ///< Actively generating step pulses
    STOPPING = 2    ///< Decelerating to stop
};

/**
 * @brief Abstract interface for step pulse generators
 *
 * All step generator implementations must inherit from this interface.
 * This enables the MotionController to work with any step generation
 * method (MCPWM, RMT, software-timed, etc.) through polymorphism.
 *
 * Responsibilities:
 * - Generate step pulses at a specified frequency
 * - Track step count for position estimation
 * - Support auto-stop after a target number of steps
 *
 * NOT responsible for:
 * - Direction control (handled by IDriver)
 * - Enable/disable control (handled by IDriver)
 * - Motion trajectory/acceleration (handled by MotionController)
 *
 * Usage:
 * @code
 *   IStepGenerator* stepper = new McpwmStepper(18, config);
 *   stepper->init();
 *   stepper->setFrequency(10000);  // 10 kHz
 *   stepper->start();
 *   // ... motor moving ...
 *   stepper->stop();
 * @endcode
 */
class IStepGenerator {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~IStepGenerator() = default;

    // ─────────────────────────────────────────────────────────────────────
    // Initialization
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Initialize the step generator hardware
     *
     * Must be called before using the step generator. Can be called
     * multiple times to reinitialize.
     *
     * @return true if initialization succeeded
     */
    virtual bool init() = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Control
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Start generating step pulses
     *
     * Frequency must be set before calling start().
     *
     * @return true if started successfully
     */
    virtual bool start() = 0;

    /**
     * @brief Stop generating step pulses immediately
     *
     * For emergency stop. For controlled deceleration, use the
     * TrajectoryInterpolator to ramp frequency down first.
     */
    virtual void stop() = 0;

    /**
     * @brief Check if currently generating steps
     * @return true if running
     */
    virtual bool isRunning() const = 0;

    /**
     * @brief Get current state
     * @return StepGenState enum value
     */
    virtual StepGenState getState() const = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Frequency Control
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Set step frequency
     *
     * Can be called while running to change speed smoothly.
     * For motion profiles, the TrajectoryInterpolator calls this
     * periodically to implement acceleration curves.
     *
     * @param frequency_hz Steps per second
     * @return true if frequency was set, false if out of range
     */
    virtual bool setFrequency(uint32_t frequency_hz) = 0;

    /**
     * @brief Get current frequency setting
     * @return Current frequency in Hz
     */
    virtual uint32_t getFrequency() const = 0;

    /**
     * @brief Get maximum supported frequency
     * @return Maximum frequency in Hz
     */
    virtual uint32_t getMaxFrequency() const = 0;

    /**
     * @brief Get minimum supported frequency
     * @return Minimum frequency in Hz
     */
    virtual uint32_t getMinFrequency() const = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Step Counting
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get steps generated since last reset
     *
     * This is an approximate count based on frequency and time.
     * For precise position tracking, use encoder feedback.
     *
     * @return Approximate step count
     */
    virtual uint32_t getStepCount() const = 0;

    /**
     * @brief Reset step counter to zero
     */
    virtual void resetStepCount() = 0;

    /**
     * @brief Set target step count for auto-stop
     *
     * When step count reaches this value, generation stops automatically.
     * Set to 0 to disable auto-stop (continuous mode).
     *
     * @param target_steps Number of steps to generate (0 = continuous)
     */
    virtual void setTargetSteps(uint32_t target_steps) = 0;

    // ─────────────────────────────────────────────────────────────────────
    // Configuration Access
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get step pin GPIO number
     * @return GPIO pin number, or -1 if not applicable
     */
    virtual int8_t getStepPin() const = 0;

    /**
     * @brief Get pulse width in microseconds
     * @return Pulse width in microseconds
     */
    virtual uint16_t getPulseWidth() const = 0;
};

}  // namespace usd

#endif  // USD_ISTEP_GENERATOR_H
