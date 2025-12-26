/**
 * @file motion_controller.h
 * @brief Open-loop motion controller with state machine
 *
 * Manages motor motion state and coordinates between driver and trajectory.
 * This is the central orchestrator for motor movement.
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#ifndef USD_MOTION_CONTROLLER_H
#define USD_MOTION_CONTROLLER_H

#include <cstdint>

// Forward declarations to avoid circular includes
namespace usd {
    class IDriver;
    class McpwmStepper;
}

namespace usd {

/**
 * @brief Motion controller operational state
 */
enum class MotionState : uint8_t {
    IDLE = 0,           ///< Stopped, ready for commands
    ACCELERATING = 1,   ///< Ramping up speed
    CRUISING = 2,       ///< At target velocity
    DECELERATING = 3,   ///< Ramping down speed
    HOLDING = 4,        ///< At position, motor enabled
    FAULT = 5,          ///< Error condition
    HOMING = 6          ///< Executing homing sequence
};

/**
 * @brief Move command type
 */
enum class MoveType : uint8_t {
    RELATIVE = 0,       ///< Move relative to current position
    ABSOLUTE = 1,       ///< Move to absolute position
    VELOCITY = 2,       ///< Constant velocity (no target position)
    HOMING = 3          ///< Homing move (seek limit switch)
};

/**
 * @brief Motion profile type
 */
enum class ProfileType : uint8_t {
    TRAPEZOIDAL = 0,    ///< Linear acceleration
    S_CURVE = 1         ///< Jerk-limited (smooth)
};

/**
 * @brief Motion parameters for a move
 */
struct MotionParams {
    int32_t target_position;     ///< Target position in steps
    uint32_t max_velocity;       ///< Max velocity in steps/sec
    uint32_t acceleration;       ///< Acceleration in steps/sec²
    uint32_t deceleration;       ///< Deceleration in steps/sec² (0 = use accel)
    uint32_t jerk;               ///< Jerk limit for S-curve (steps/sec³)
    ProfileType profile;         ///< Profile type to use
    MoveType move_type;          ///< Type of move
};

/**
 * @brief Current motion status
 */
struct MotionStatus {
    MotionState state;           ///< Current state
    int32_t current_position;    ///< Current position in steps
    int32_t target_position;     ///< Target position in steps
    uint32_t current_velocity;   ///< Current velocity in steps/sec
    uint32_t distance_to_go;     ///< Steps remaining to target
    bool in_motion;              ///< True if motor is moving
    bool at_target;              ///< True if at target position
};

/**
 * @brief Motion controller configuration
 */
struct MotionConfig {
    uint32_t default_velocity;       ///< Default max velocity (steps/sec)
    uint32_t default_acceleration;   ///< Default acceleration (steps/sec²)
    uint32_t min_velocity;           ///< Minimum stepping velocity
    uint32_t position_tolerance;     ///< Steps within target = "at position"
    bool enable_on_motion;           ///< Auto-enable driver on move
    bool disable_on_idle;            ///< Auto-disable after hold time
    uint32_t idle_disable_ms;        ///< Time before auto-disable (ms)
};

/**
 * @brief Open-loop motion controller
 *
 * Manages a single axis of motion including:
 * - State machine for motion phases (accel/cruise/decel)
 * - Trajectory generation (trapezoidal/S-curve)
 * - Driver integration (step generation)
 * - Position tracking
 *
 * This is an OPEN-LOOP controller. For closed-loop control with
 * encoder feedback, use ClosedLoopController (Phase P3).
 *
 * Thread Safety:
 * - All public methods are thread-safe via internal mutex
 * - tick() should be called from MotionTask at regular intervals
 *
 * Usage:
 * @code
 *   MotionController controller;
 *   controller.attachDriver(&driver);
 *   controller.attachStepper(&stepper);
 *   controller.enable();
 *
 *   MotionParams params = {
 *       .target_position = 10000,
 *       .max_velocity = 50000,
 *       .acceleration = 100000,
 *       .profile = ProfileType::TRAPEZOIDAL,
 *       .move_type = MoveType::ABSOLUTE
 *   };
 *   controller.startMove(params);
 *
 *   while (!controller.isAtTarget()) {
 *       controller.tick();  // Called from MotionTask
 *   }
 * @endcode
 */
class MotionController {
public:
    /**
     * @brief Construct motion controller with default configuration
     */
    MotionController();

    /**
     * @brief Construct with custom configuration
     * @param config Motion configuration
     */
    explicit MotionController(const MotionConfig& config);

    /**
     * @brief Destructor
     */
    ~MotionController();

    // ─────────────────────────────────────────────────────────────────────
    // Hardware Attachment
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Attach motor driver for enable/direction control
     * @param driver Pointer to IDriver implementation
     */
    void attachDriver(IDriver* driver);

    /**
     * @brief Attach MCPWM stepper for step pulse generation
     * @param stepper Pointer to McpwmStepper
     */
    void attachStepper(McpwmStepper* stepper);

    /**
     * @brief Check if hardware is attached
     * @return true if both driver and stepper are attached
     */
    bool isHardwareAttached() const;

    // ─────────────────────────────────────────────────────────────────────
    // Enable/Disable
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Enable the motor driver
     * @return true if enabled successfully
     */
    bool enable();

    /**
     * @brief Disable the motor driver
     */
    void disable();

    /**
     * @brief Check if driver is enabled
     */
    bool isEnabled() const;

    // ─────────────────────────────────────────────────────────────────────
    // Motion Commands
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Start a move with specified parameters
     *
     * This is the main motion command. The controller will:
     * 1. Enable driver (if configured)
     * 2. Set direction
     * 3. Calculate trajectory
     * 4. Start stepping
     *
     * @param params Motion parameters
     * @return true if move started, false if error
     */
    bool startMove(const MotionParams& params);

    /**
     * @brief Move to absolute position with default velocity
     * @param position Target position in steps
     * @return true if move started
     */
    bool moveTo(int32_t position);

    /**
     * @brief Move relative to current position
     * @param distance Steps to move (positive or negative)
     * @return true if move started
     */
    bool moveBy(int32_t distance);

    /**
     * @brief Start constant velocity motion
     * @param velocity Velocity in steps/sec (negative for reverse)
     * @return true if started
     */
    bool startVelocity(int32_t velocity);

    /**
     * @brief Stop motion immediately (no deceleration)
     *
     * Emergency stop. Motor may lose position.
     */
    void emergencyStop();

    /**
     * @brief Stop motion with deceleration profile
     *
     * Decelerates to stop using current profile.
     */
    void stop();

    // ─────────────────────────────────────────────────────────────────────
    // State & Status
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get current motion state
     */
    MotionState getState() const;

    /**
     * @brief Get full motion status
     */
    MotionStatus getStatus() const;

    /**
     * @brief Check if motor is in motion
     */
    bool isMoving() const;

    /**
     * @brief Check if at target position (within tolerance)
     */
    bool isAtTarget() const;

    /**
     * @brief Get current position in steps
     */
    int32_t getPosition() const;

    /**
     * @brief Get current velocity in steps/sec
     */
    uint32_t getVelocity() const;

    /**
     * @brief Get remaining distance to target
     */
    int32_t getDistanceToGo() const;

    // ─────────────────────────────────────────────────────────────────────
    // Position Management
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Set current position (for homing)
     * @param position New position value
     */
    void setPosition(int32_t position);

    /**
     * @brief Reset position to zero
     */
    void resetPosition();

    // ─────────────────────────────────────────────────────────────────────
    // Configuration
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Set motion configuration
     */
    void setConfig(const MotionConfig& config);

    /**
     * @brief Get current configuration
     */
    const MotionConfig& getConfig() const;

    /**
     * @brief Set default velocity for simple moves
     */
    void setMaxVelocity(uint32_t velocity);

    /**
     * @brief Set default acceleration
     */
    void setAcceleration(uint32_t acceleration);

    // ─────────────────────────────────────────────────────────────────────
    // Control Loop
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Main control loop tick
     *
     * Must be called regularly from MotionTask (e.g., 1-10 kHz).
     * Updates trajectory, adjusts velocity, manages state transitions.
     *
     * @param dt_us Delta time in microseconds since last tick
     */
    void tick(uint32_t dt_us);

private:
    IDriver* driver_;
    McpwmStepper* stepper_;
    MotionConfig config_;
    MotionState state_;

    int32_t current_position_;
    int32_t target_position_;
    uint32_t current_velocity_;
    uint32_t target_velocity_;

    MotionParams active_params_;
    uint32_t motion_start_time_;
    uint32_t last_tick_time_;

    bool enabled_;

    /**
     * @brief Transition to new state
     */
    void setState(MotionState new_state);

    /**
     * @brief Calculate trajectory for current move
     */
    void calculateTrajectory();

    /**
     * @brief Update velocity based on trajectory phase
     */
    void updateVelocity(uint32_t dt_us);

    /**
     * @brief Check for state transitions
     */
    void checkTransitions();

    /**
     * @brief Apply current velocity to stepper
     */
    void applyVelocity();
};

/**
 * @brief Get string representation of motion state
 */
const char* motionStateToString(MotionState state);

}  // namespace usd

#endif  // USD_MOTION_CONTROLLER_H
