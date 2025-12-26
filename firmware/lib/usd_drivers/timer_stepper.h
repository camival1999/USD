/**
 * @file timer_stepper.h
 * @brief Timer-based step pulse generator for ESP32 (fallback implementation)
 *
 * Uses ESP32 hardware timer with ISR to generate step pulses. This is a
 * fallback implementation for platforms/cases where MCPWM is unavailable.
 * Lower maximum frequency than MCPWM but still provides hardware timing.
 *
 * Key features:
 * - Uses esp_timer for microsecond-precision timing
 * - Maximum 50kHz (vs MCPWM's 500kHz) due to ISR overhead
 * - Same IStepGenerator interface for interchangeable use
 *
 * @version 0.1.0
 * @date 2025-05-28
 */

#ifndef USD_TIMER_STEPPER_H
#define USD_TIMER_STEPPER_H

#include "istep_generator.h"

#ifdef ARDUINO
#include <esp32-hal.h>
#include "esp_timer.h"
#endif

namespace usd {

/**
 * @brief Timer configuration for TimerStepper
 */
struct TimerConfig {
    const char* timer_name;  ///< Name for the esp_timer (for debugging)
    bool skip_unhandled;     ///< Skip timer callbacks if previous hasn't finished
    
    /**
     * @brief Default configuration
     */
    static TimerConfig defaultConfig() {
        return TimerConfig{
            .timer_name = "step_timer",
            .skip_unhandled = true
        };
    }
};

/**
 * @brief Timer-based step pulse generator (ISR fallback)
 *
 * This class uses the ESP32 esp_timer API to generate step pulses via
 * timer interrupt. It's a fallback for when MCPWM isn't available or
 * when a simpler implementation is preferred.
 *
 * Performance characteristics:
 * - Max frequency: 50 kHz (limited by ISR overhead)
 * - Timing accuracy: ~1 microsecond
 * - CPU impact: Higher than MCPWM due to ISR execution
 *
 * The timer ISR toggles the GPIO to create step pulses. Each pulse
 * consists of a rising edge (step trigger) and falling edge (reset).
 *
 * Thread Safety:
 * - setFrequency() and stop() can be called from any task
 * - start() should be called from MotionTask only
 * - Step counting is atomic
 *
 * Usage:
 * @code
 *   TimerConfig config = TimerConfig::defaultConfig();
 *   TimerStepper stepper(18, config);  // Step pin GPIO 18
 *
 *   stepper.init();
 *   stepper.setFrequency(10000);  // 10 kHz
 *   stepper.start();
 *   // ... motor moving ...
 *   stepper.stop();
 * @endcode
 */
class TimerStepper : public IStepGenerator {
public:
    /**
     * @brief Construct timer-based step generator
     *
     * @param step_pin GPIO pin for step output
     * @param config Timer configuration
     * @param pulse_width_us Step pulse width in microseconds (default 3)
     */
    TimerStepper(int8_t step_pin, const TimerConfig& config = TimerConfig::defaultConfig(), 
                 uint16_t pulse_width_us = 3);

    /**
     * @brief Destructor - stops timer and releases resources
     */
    ~TimerStepper();

    // ─────────────────────────────────────────────────────────────────────
    // Control
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Initialize timer peripheral
     *
     * Must be called before using the stepper. Can be called multiple
     * times to reinitialize.
     *
     * @return true if initialization succeeded
     */
    bool init() override;

    /**
     * @brief Start generating step pulses
     *
     * Frequency must be set before calling start().
     *
     * @return true if started successfully
     */
    bool start() override;

    /**
     * @brief Stop generating step pulses immediately
     *
     * For emergency stop. For controlled deceleration, use the
     * TrajectoryInterpolator to ramp frequency down first.
     */
    void stop() override;

    /**
     * @brief Check if currently generating steps
     * @return true if running
     */
    bool isRunning() const override;

    /**
     * @brief Get current state
     * @return StepGenState enum value
     */
    StepGenState getState() const override;

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
     * @param frequency_hz Steps per second (1 Hz to 50 kHz)
     * @return true if frequency was set, false if out of range
     */
    bool setFrequency(uint32_t frequency_hz) override;

    /**
     * @brief Get current frequency setting
     * @return Current frequency in Hz
     */
    uint32_t getFrequency() const override;

    /**
     * @brief Get maximum supported frequency
     * @return Maximum frequency in Hz (50 kHz for timer-based)
     */
    uint32_t getMaxFrequency() const override { return MAX_FREQUENCY; }

    /**
     * @brief Get minimum supported frequency
     * @return Minimum frequency in Hz (1 Hz)
     */
    uint32_t getMinFrequency() const override { return MIN_FREQUENCY; }

    // ─────────────────────────────────────────────────────────────────────
    // Step Counting
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get steps generated since last reset
     *
     * This is an accurate count from the ISR.
     *
     * @return Step count
     */
    uint32_t getStepCount() const override;

    /**
     * @brief Reset step counter to zero
     */
    void resetStepCount() override;

    /**
     * @brief Set target step count for auto-stop
     *
     * When step count reaches this value, timer stops automatically.
     * Set to 0 to disable auto-stop (continuous mode).
     *
     * @param target_steps Number of steps to generate (0 = continuous)
     */
    void setTargetSteps(uint32_t target_steps) override;

    // ─────────────────────────────────────────────────────────────────────
    // Configuration
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get step pin GPIO number
     */
    int8_t getStepPin() const override { return step_pin_; }

    /**
     * @brief Get timer configuration
     */
    const TimerConfig& getConfig() const { return config_; }

    /**
     * @brief Get pulse width in microseconds
     */
    uint16_t getPulseWidth() const override { return pulse_width_us_; }

private:
    static constexpr uint32_t MIN_FREQUENCY = 1;      ///< 1 Hz minimum
    static constexpr uint32_t MAX_FREQUENCY = 50000;  ///< 50 kHz maximum (ISR overhead limit)

    int8_t step_pin_;
    TimerConfig config_;
    uint16_t pulse_width_us_;
    
    StepGenState state_;
    uint32_t frequency_hz_;
    
    volatile uint32_t step_count_;
    uint32_t target_steps_;
    
    bool initialized_;
    
    // Pulse state tracking for two-phase toggle (high then low)
    volatile bool pulse_high_;

#ifdef ARDUINO
    esp_timer_handle_t step_timer_;
    esp_timer_handle_t pulse_end_timer_;  // For ending the pulse after pulse_width_us
    
    /**
     * @brief Timer ISR callback for step pulse generation
     * @param arg Pointer to TimerStepper instance
     */
    static void IRAM_ATTR stepTimerCallback(void* arg);
    
    /**
     * @brief Timer callback to end the pulse (bring GPIO low)
     * @param arg Pointer to TimerStepper instance
     */
    static void IRAM_ATTR pulseEndCallback(void* arg);
#endif

    /**
     * @brief Calculate timer period from frequency
     * @param freq_hz Frequency in Hz
     * @return Period in microseconds
     */
    uint64_t calculatePeriodUs(uint32_t freq_hz) const;
    
    /**
     * @brief Update timer period when frequency changes
     */
    void updateTimerPeriod();
};

}  // namespace usd

#endif  // USD_TIMER_STEPPER_H
