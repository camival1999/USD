/**
 * @file motion_controller.cpp
 * @brief Open-loop motion controller implementation
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#include "motion_controller.h"
#include "idriver.h"
#include "mcpwm_stepper.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <chrono>
static uint32_t millis() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return static_cast<uint32_t>(ms.count());
}
#endif

namespace usd {

// ─────────────────────────────────────────────────────────────────────────────
// Default Configuration
// ─────────────────────────────────────────────────────────────────────────────

static const MotionConfig DEFAULT_CONFIG = {
    .default_velocity = 10000,       // 10 kHz default
    .default_acceleration = 50000,   // 50k steps/sec²
    .min_velocity = 100,             // 100 Hz minimum
    .position_tolerance = 1,         // 1 step tolerance
    .enable_on_motion = true,        // Auto-enable
    .disable_on_idle = false,        // Don't auto-disable
    .idle_disable_ms = 5000          // 5 second timeout
};

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

MotionController::MotionController()
    : driver_(nullptr)
    , stepper_(nullptr)
    , config_(DEFAULT_CONFIG)
    , state_(MotionState::IDLE)
    , current_position_(0)
    , target_position_(0)
    , current_velocity_(0)
    , target_velocity_(0)
    , active_params_{}
    , motion_start_time_(0)
    , last_tick_time_(0)
    , enabled_(false)
{
}

MotionController::MotionController(const MotionConfig& config)
    : driver_(nullptr)
    , stepper_(nullptr)
    , config_(config)
    , state_(MotionState::IDLE)
    , current_position_(0)
    , target_position_(0)
    , current_velocity_(0)
    , target_velocity_(0)
    , active_params_{}
    , motion_start_time_(0)
    , last_tick_time_(0)
    , enabled_(false)
{
}

MotionController::~MotionController()
{
    emergencyStop();
}

// ─────────────────────────────────────────────────────────────────────────────
// Hardware Attachment
// ─────────────────────────────────────────────────────────────────────────────

void MotionController::attachDriver(IDriver* driver)
{
    driver_ = driver;
}

void MotionController::attachStepper(McpwmStepper* stepper)
{
    stepper_ = stepper;
}

bool MotionController::isHardwareAttached() const
{
    return driver_ != nullptr && stepper_ != nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Enable/Disable
// ─────────────────────────────────────────────────────────────────────────────

bool MotionController::enable()
{
    if (!driver_) {
        return false;
    }

    if (driver_->enable()) {
        enabled_ = true;
        return true;
    }
    return false;
}

void MotionController::disable()
{
    if (state_ != MotionState::IDLE) {
        emergencyStop();
    }

    if (driver_) {
        driver_->disable();
    }
    enabled_ = false;
}

bool MotionController::isEnabled() const
{
    return enabled_ && driver_ && driver_->isEnabled();
}

// ─────────────────────────────────────────────────────────────────────────────
// Motion Commands
// ─────────────────────────────────────────────────────────────────────────────

bool MotionController::startMove(const MotionParams& params)
{
    if (!isHardwareAttached()) {
        return false;
    }

    // Auto-enable if configured
    if (config_.enable_on_motion && !isEnabled()) {
        if (!enable()) {
            return false;
        }
    }

    // Store parameters
    active_params_ = params;

    // Calculate target based on move type
    switch (params.move_type) {
        case MoveType::ABSOLUTE:
            target_position_ = params.target_position;
            break;
        case MoveType::RELATIVE:
            target_position_ = current_position_ + params.target_position;
            break;
        case MoveType::VELOCITY:
            // Velocity mode - no target position
            target_velocity_ = params.max_velocity;
            break;
        case MoveType::HOMING:
            // Homing handled separately
            return false;  // TODO: Implement homing
    }

    // Set direction
    if (target_position_ > current_position_) {
        driver_->setDirection(Direction::CW);
    } else if (target_position_ < current_position_) {
        driver_->setDirection(Direction::CCW);
    } else {
        // Already at target
        setState(MotionState::HOLDING);
        return true;
    }

    // Calculate trajectory
    target_velocity_ = params.max_velocity;
    calculateTrajectory();

    // Start motion
    motion_start_time_ = millis();
    setState(MotionState::ACCELERATING);

    // Start step generator
    if (stepper_) {
        stepper_->setFrequency(config_.min_velocity);
        stepper_->start();
    }

    return true;
}

bool MotionController::moveTo(int32_t position)
{
    MotionParams params = {
        .target_position = position,
        .max_velocity = config_.default_velocity,
        .acceleration = config_.default_acceleration,
        .deceleration = config_.default_acceleration,
        .jerk = 0,
        .profile = ProfileType::TRAPEZOIDAL,
        .move_type = MoveType::ABSOLUTE
    };
    return startMove(params);
}

bool MotionController::moveBy(int32_t distance)
{
    MotionParams params = {
        .target_position = distance,
        .max_velocity = config_.default_velocity,
        .acceleration = config_.default_acceleration,
        .deceleration = config_.default_acceleration,
        .jerk = 0,
        .profile = ProfileType::TRAPEZOIDAL,
        .move_type = MoveType::RELATIVE
    };
    return startMove(params);
}

bool MotionController::startVelocity(int32_t velocity)
{
    if (!isHardwareAttached()) {
        return false;
    }

    if (config_.enable_on_motion && !isEnabled()) {
        if (!enable()) {
            return false;
        }
    }

    // Set direction based on velocity sign
    if (velocity > 0) {
        driver_->setDirection(Direction::CW);
        target_velocity_ = static_cast<uint32_t>(velocity);
    } else if (velocity < 0) {
        driver_->setDirection(Direction::CCW);
        target_velocity_ = static_cast<uint32_t>(-velocity);
    } else {
        stop();
        return true;
    }

    active_params_.max_velocity = target_velocity_;
    active_params_.move_type = MoveType::VELOCITY;

    motion_start_time_ = millis();
    setState(MotionState::ACCELERATING);

    if (stepper_) {
        stepper_->setFrequency(config_.min_velocity);
        stepper_->start();
    }

    return true;
}

void MotionController::emergencyStop()
{
    if (stepper_) {
        stepper_->stop();
    }
    current_velocity_ = 0;
    setState(MotionState::IDLE);
}

void MotionController::stop()
{
    if (state_ == MotionState::IDLE || state_ == MotionState::HOLDING) {
        return;
    }

    // Start deceleration
    setState(MotionState::DECELERATING);
    target_velocity_ = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// State & Status
// ─────────────────────────────────────────────────────────────────────────────

MotionState MotionController::getState() const
{
    return state_;
}

MotionStatus MotionController::getStatus() const
{
    MotionStatus status;
    status.state = state_;
    status.current_position = current_position_;
    status.target_position = target_position_;
    status.current_velocity = current_velocity_;

    int32_t distance = target_position_ - current_position_;
    status.distance_to_go = distance >= 0 ? distance : -distance;

    status.in_motion = (state_ == MotionState::ACCELERATING ||
                        state_ == MotionState::CRUISING ||
                        state_ == MotionState::DECELERATING);

    status.at_target = (status.distance_to_go <= config_.position_tolerance);

    return status;
}

bool MotionController::isMoving() const
{
    return state_ == MotionState::ACCELERATING ||
           state_ == MotionState::CRUISING ||
           state_ == MotionState::DECELERATING;
}

bool MotionController::isAtTarget() const
{
    int32_t distance = target_position_ - current_position_;
    if (distance < 0) distance = -distance;
    return static_cast<uint32_t>(distance) <= config_.position_tolerance;
}

int32_t MotionController::getPosition() const
{
    return current_position_;
}

uint32_t MotionController::getVelocity() const
{
    return current_velocity_;
}

int32_t MotionController::getDistanceToGo() const
{
    return target_position_ - current_position_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Position Management
// ─────────────────────────────────────────────────────────────────────────────

void MotionController::setPosition(int32_t position)
{
    current_position_ = position;
    if (driver_) {
        driver_->setPosition(position);
    }
}

void MotionController::resetPosition()
{
    setPosition(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────────────────────

void MotionController::setConfig(const MotionConfig& config)
{
    config_ = config;
}

const MotionConfig& MotionController::getConfig() const
{
    return config_;
}

void MotionController::setMaxVelocity(uint32_t velocity)
{
    config_.default_velocity = velocity;
}

void MotionController::setAcceleration(uint32_t acceleration)
{
    config_.default_acceleration = acceleration;
}

// ─────────────────────────────────────────────────────────────────────────────
// Control Loop
// ─────────────────────────────────────────────────────────────────────────────

void MotionController::tick(uint32_t dt_us)
{
    if (state_ == MotionState::IDLE || state_ == MotionState::FAULT) {
        return;
    }

    // Update velocity based on trajectory phase
    updateVelocity(dt_us);

    // Apply velocity to step generator
    applyVelocity();

    // Update position from driver (if using software stepping)
    if (driver_) {
        current_position_ = driver_->getPosition();
    }

    // Check for state transitions
    checkTransitions();
}

// ─────────────────────────────────────────────────────────────────────────────
// Private Methods
// ─────────────────────────────────────────────────────────────────────────────

void MotionController::setState(MotionState new_state)
{
    state_ = new_state;
}

void MotionController::calculateTrajectory()
{
    // For trapezoidal profile:
    // Calculate accel/decel distances and times
    // This is a simplified implementation - full trajectory planning
    // is handled by TrajectoryInterpolator in P1.6

    // For now, just set target velocity
    target_velocity_ = active_params_.max_velocity;
}

void MotionController::updateVelocity(uint32_t dt_us)
{
    if (dt_us == 0) return;

    uint32_t accel = active_params_.acceleration;
    if (accel == 0) accel = config_.default_acceleration;

    // Convert dt from microseconds to seconds (fixed-point)
    // acceleration is in steps/sec², dt is in µs
    // delta_v = a * dt = a * (dt_us / 1000000)
    // To avoid floating point: delta_v = (a * dt_us) / 1000000

    uint64_t delta_v = (static_cast<uint64_t>(accel) * dt_us) / 1000000;

    switch (state_) {
        case MotionState::ACCELERATING:
            if (current_velocity_ < target_velocity_) {
                current_velocity_ += static_cast<uint32_t>(delta_v);
                if (current_velocity_ > target_velocity_) {
                    current_velocity_ = target_velocity_;
                }
            }
            break;

        case MotionState::DECELERATING:
            if (current_velocity_ > config_.min_velocity) {
                if (delta_v < current_velocity_ - config_.min_velocity) {
                    current_velocity_ -= static_cast<uint32_t>(delta_v);
                } else {
                    current_velocity_ = config_.min_velocity;
                }
            }
            break;

        case MotionState::CRUISING:
            // Maintain velocity
            current_velocity_ = target_velocity_;
            break;

        default:
            break;
    }

    // Clamp to valid range
    if (current_velocity_ < config_.min_velocity && isMoving()) {
        current_velocity_ = config_.min_velocity;
    }
}

void MotionController::checkTransitions()
{
    int32_t distance = target_position_ - current_position_;
    if (distance < 0) distance = -distance;
    uint32_t abs_distance = static_cast<uint32_t>(distance);

    switch (state_) {
        case MotionState::ACCELERATING:
            if (current_velocity_ >= target_velocity_) {
                // Reached cruise velocity
                setState(MotionState::CRUISING);
            }
            // Check if need to start decelerating
            // (simplified: decel distance = v² / (2 * a))
            if (active_params_.move_type != MoveType::VELOCITY) {
                uint64_t v = current_velocity_;
                uint64_t a = active_params_.acceleration;
                if (a == 0) a = config_.default_acceleration;
                uint64_t decel_distance = (v * v) / (2 * a);

                if (abs_distance <= decel_distance) {
                    setState(MotionState::DECELERATING);
                }
            }
            break;

        case MotionState::CRUISING:
            // Check if need to start decelerating
            if (active_params_.move_type != MoveType::VELOCITY) {
                uint64_t v = current_velocity_;
                uint64_t a = active_params_.acceleration;
                if (a == 0) a = config_.default_acceleration;
                uint64_t decel_distance = (v * v) / (2 * a);

                if (abs_distance <= decel_distance) {
                    setState(MotionState::DECELERATING);
                }
            }
            break;

        case MotionState::DECELERATING:
            if (abs_distance <= config_.position_tolerance) {
                // At target
                if (stepper_) {
                    stepper_->stop();
                }
                current_velocity_ = 0;
                setState(MotionState::HOLDING);
            } else if (current_velocity_ <= config_.min_velocity) {
                // Stopped but not at target - overshoot protection
                if (stepper_) {
                    stepper_->stop();
                }
                current_velocity_ = 0;
                setState(MotionState::IDLE);
            }
            break;

        case MotionState::HOLDING:
            // Check for auto-disable timeout
            // (not implemented in this version)
            break;

        default:
            break;
    }
}

void MotionController::applyVelocity()
{
    if (stepper_ && isMoving()) {
        if (current_velocity_ >= config_.min_velocity) {
            stepper_->setFrequency(current_velocity_);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility Functions
// ─────────────────────────────────────────────────────────────────────────────

const char* motionStateToString(MotionState state)
{
    switch (state) {
        case MotionState::IDLE:         return "IDLE";
        case MotionState::ACCELERATING: return "ACCELERATING";
        case MotionState::CRUISING:     return "CRUISING";
        case MotionState::DECELERATING: return "DECELERATING";
        case MotionState::HOLDING:      return "HOLDING";
        case MotionState::FAULT:        return "FAULT";
        case MotionState::HOMING:       return "HOMING";
        default:                        return "UNKNOWN";
    }
}

}  // namespace usd
