/**
 * @file mcpwm_stepper.h
 * @brief MCPWM-based step pulse generator for ESP32
 *
 * Uses the ESP32's Motor Control PWM (MCPWM) peripheral to generate
 * precise, hardware-timed step pulses at high frequencies (up to 500kHz).
 *
 * Key features:
 * - Hardware timing eliminates jitter from task scheduling
 * - Automatic frequency adjustment during motion
 * - Integrates with IDriver for seamless switching
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#ifndef USD_MCPWM_STEPPER_H
#define USD_MCPWM_STEPPER_H

#include "idriver.h"

#ifdef ARDUINO
#include <esp32-hal.h>
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#endif

namespace usd {

/**
 * @brief MCPWM unit configuration
 */
struct McpwmConfig {
    uint8_t unit;          ///< MCPWM unit (0 or 1 on ESP32-S3)
    uint8_t timer;         ///< Timer within unit (0, 1, or 2)
    uint8_t operator_id;   ///< Operator (0 or 1)
    uint8_t output;        ///< Output A or B (MCPWM_OPR_A = 0, MCPWM_OPR_B = 1)
};

/**
 * @brief Step generator state
 */
enum class StepGenState : uint8_t {
    IDLE = 0,       ///< Not generating steps
    RUNNING = 1,    ///< Actively generating step pulses
    STOPPING = 2    ///< Decelerating to stop
};

/**
 * @brief MCPWM-based step pulse generator
 *
 * This class wraps the ESP32 MCPWM peripheral to generate step pulses
 * with hardware-level timing accuracy. It works alongside an IDriver
 * implementation (for direction and enable control).
 *
 * The MCPWM generates a precise PWM signal where each pulse is a step.
 * The frequency directly determines stepping speed.
 *
 * Thread Safety:
 * - setFrequency() and stop() can be called from any task
 * - start() should be called from MotionTask only
 * - Internal state is protected by critical sections
 *
 * Usage:
 * @code
 *   McpwmConfig config = {
 *       .unit = 0,
 *       .timer = 0,
 *       .operator_id = 0,
 *       .output = 0
 *   };
 *   McpwmStepper stepper(18, config);  // Step pin GPIO 18
 *
 *   stepper.setFrequency(10000);  // 10 kHz
 *   stepper.start();
 *   // ... motor moving ...
 *   stepper.stop();
 * @endcode
 */
class McpwmStepper {
public:
    /**
     * @brief Construct MCPWM step generator
     *
     * @param step_pin GPIO pin for step output
     * @param config MCPWM unit/timer/operator configuration
     * @param pulse_width_us Step pulse width in microseconds (default 3)
     */
    McpwmStepper(int8_t step_pin, const McpwmConfig& config, uint16_t pulse_width_us = 3);

    /**
     * @brief Destructor - stops PWM and releases resources
     */
    ~McpwmStepper();

    // ─────────────────────────────────────────────────────────────────────
    // Control
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Initialize MCPWM peripheral
     *
     * Must be called before using the stepper. Can be called multiple
     * times to reinitialize.
     *
     * @return true if initialization succeeded
     */
    bool init();

    /**
     * @brief Start generating step pulses
     *
     * Frequency must be set before calling start().
     *
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop generating step pulses immediately
     *
     * For emergency stop. For controlled deceleration, use the
     * TrajectoryInterpolator to ramp frequency down first.
     */
    void stop();

    /**
     * @brief Check if currently generating steps
     * @return true if running
     */
    bool isRunning() const;

    /**
     * @brief Get current state
     * @return StepGenState enum value
     */
    StepGenState getState() const;

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
     * @param frequency_hz Steps per second (1 Hz to 500 kHz)
     * @return true if frequency was set, false if out of range
     */
    bool setFrequency(uint32_t frequency_hz);

    /**
     * @brief Get current frequency setting
     * @return Current frequency in Hz
     */
    uint32_t getFrequency() const;

    /**
     * @brief Get maximum supported frequency
     * @return Maximum frequency in Hz (typically 500 kHz)
     */
    static uint32_t getMaxFrequency() { return MAX_FREQUENCY; }

    /**
     * @brief Get minimum supported frequency
     * @return Minimum frequency in Hz (typically 1 Hz)
     */
    static uint32_t getMinFrequency() { return MIN_FREQUENCY; }

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
    uint32_t getStepCount() const;

    /**
     * @brief Reset step counter to zero
     */
    void resetStepCount();

    /**
     * @brief Set target step count for auto-stop
     *
     * When step count reaches this value, PWM stops automatically.
     * Set to 0 to disable auto-stop (continuous mode).
     *
     * @param target_steps Number of steps to generate (0 = continuous)
     */
    void setTargetSteps(uint32_t target_steps);

    // ─────────────────────────────────────────────────────────────────────
    // Configuration
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get step pin GPIO number
     */
    int8_t getStepPin() const { return step_pin_; }

    /**
     * @brief Get MCPWM configuration
     */
    const McpwmConfig& getConfig() const { return config_; }

    /**
     * @brief Get pulse width in microseconds
     */
    uint16_t getPulseWidth() const { return pulse_width_us_; }

private:
    static constexpr uint32_t MIN_FREQUENCY = 1;       // 1 Hz
    static constexpr uint32_t MAX_FREQUENCY = 500000;  // 500 kHz

    int8_t step_pin_;
    McpwmConfig config_;
    uint16_t pulse_width_us_;
    
    StepGenState state_;
    uint32_t frequency_hz_;
    
    volatile uint32_t step_count_;
    uint32_t target_steps_;
    
    bool initialized_;

    /**
     * @brief Configure PWM duty cycle for desired pulse width
     */
    void updateDutyCycle();

    /**
     * @brief Calculate duty cycle percentage for given frequency
     * @param freq_hz Frequency in Hz
     * @return Duty cycle percentage (0.0 - 100.0)
     */
    float calculateDuty(uint32_t freq_hz) const;
};

}  // namespace usd

#endif  // USD_MCPWM_STEPPER_H
