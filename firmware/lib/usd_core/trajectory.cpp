/**
 * @file trajectory.cpp
 * @brief Trajectory generation implementation
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#include "trajectory.h"
#include <cmath>

namespace usd {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

TrapezoidalTrajectory::TrapezoidalTrajectory()
    : params_{}
    , timing_{}
    , phase_(TrajectoryPhase::IDLE)
    , elapsed_us_(0)
    , current_position_(0)
    , current_velocity_(0)
    , planned_(false)
    , started_(false)
    , direction_(1)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Planning
// ─────────────────────────────────────────────────────────────────────────────

bool TrapezoidalTrajectory::plan(const TrajectoryParams& params)
{
    // Validate parameters
    if (params.max_velocity == 0 || params.acceleration == 0) {
        return false;
    }

    params_ = params;

    // Use acceleration for deceleration if not specified
    if (params_.deceleration == 0) {
        params_.deceleration = params_.acceleration;
    }

    // Determine direction
    if (params_.distance >= 0) {
        direction_ = 1;
    } else {
        direction_ = -1;
        params_.distance = -params_.distance;  // Work with positive distance
    }

    // Calculate if we can reach max velocity
    // Distance to accelerate: d_accel = v² / (2a)
    // Distance to decelerate: d_decel = v² / (2d)
    uint64_t v = params_.max_velocity;
    uint64_t a = params_.acceleration;
    uint64_t d = params_.deceleration;

    uint64_t accel_dist = (v * v) / (2 * a);
    uint64_t decel_dist = (v * v) / (2 * d);

    if (accel_dist + decel_dist <= static_cast<uint64_t>(params_.distance)) {
        // Full trapezoidal profile
        computeTrapezoidal();
    } else {
        // Triangular profile - can't reach max velocity
        computeTriangular();
    }

    planned_ = true;
    started_ = false;
    phase_ = TrajectoryPhase::IDLE;

    return true;
}

void TrapezoidalTrajectory::computeTrapezoidal()
{
    uint64_t v = params_.max_velocity;
    uint64_t a = params_.acceleration;
    uint64_t d = params_.deceleration;

    // Acceleration phase
    // d = v² / (2a)
    // t = v / a
    timing_.accel_distance = static_cast<int32_t>((v * v) / (2 * a));
    timing_.accel_time_us = static_cast<uint32_t>((v * 1000000ULL) / a);

    // Deceleration phase
    timing_.decel_distance = static_cast<int32_t>((v * v) / (2 * d));
    timing_.decel_time_us = static_cast<uint32_t>((v * 1000000ULL) / d);

    // Cruise phase (remaining distance)
    timing_.cruise_distance = params_.distance - timing_.accel_distance - timing_.decel_distance;
    
    // Time at cruise = distance / velocity
    if (v > 0) {
        timing_.cruise_time_us = static_cast<uint32_t>(
            (static_cast<uint64_t>(timing_.cruise_distance) * 1000000ULL) / v);
    } else {
        timing_.cruise_time_us = 0;
    }

    timing_.total_time_us = timing_.accel_time_us + timing_.cruise_time_us + timing_.decel_time_us;
    timing_.peak_velocity = params_.max_velocity;
    timing_.is_triangle = false;
}

void TrapezoidalTrajectory::computeTriangular()
{
    // For triangular profile, we need to find the peak velocity
    // that allows us to accel and decel within the distance
    //
    // d_total = d_accel + d_decel
    // d_accel = v_peak² / (2a)
    // d_decel = v_peak² / (2d)
    // d_total = v_peak² * (1/(2a) + 1/(2d))
    // v_peak = sqrt(d_total / (1/(2a) + 1/(2d)))
    // v_peak = sqrt(2 * d_total * a * d / (a + d))

    uint64_t dist = static_cast<uint64_t>(params_.distance);
    uint64_t a = params_.acceleration;
    uint64_t d = params_.deceleration;

    // v_peak² = 2 * d * a * d / (a + d)
    double v_peak_sq = (2.0 * dist * a * d) / (a + d);
    uint32_t v_peak = static_cast<uint32_t>(sqrt(v_peak_sq));

    if (v_peak > params_.max_velocity) {
        v_peak = params_.max_velocity;
    }

    // Recalculate with actual peak velocity
    timing_.peak_velocity = v_peak;

    // Acceleration phase
    uint64_t v = v_peak;
    timing_.accel_distance = static_cast<int32_t>((v * v) / (2 * a));
    timing_.accel_time_us = static_cast<uint32_t>((v * 1000000ULL) / a);

    // Deceleration phase
    timing_.decel_distance = params_.distance - timing_.accel_distance;
    timing_.decel_time_us = static_cast<uint32_t>((v * 1000000ULL) / d);

    // No cruise phase
    timing_.cruise_distance = 0;
    timing_.cruise_time_us = 0;

    timing_.total_time_us = timing_.accel_time_us + timing_.decel_time_us;
    timing_.is_triangle = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Execution
// ─────────────────────────────────────────────────────────────────────────────

void TrapezoidalTrajectory::start()
{
    if (!planned_) {
        return;
    }

    elapsed_us_ = 0;
    current_position_ = 0;
    current_velocity_ = params_.start_velocity;
    phase_ = TrajectoryPhase::ACCEL;
    started_ = true;
}

TrajectoryState TrapezoidalTrajectory::update(uint32_t dt_us)
{
    TrajectoryState state;

    if (!started_ || phase_ == TrajectoryPhase::IDLE || phase_ == TrajectoryPhase::COMPLETE) {
        state.phase = phase_;
        state.elapsed_us = elapsed_us_;
        state.position = current_position_;
        state.velocity = 0;
        state.remaining = params_.distance - current_position_;
        state.progress = (params_.distance > 0) ? 
            static_cast<float>(current_position_) / params_.distance : 1.0f;
        return state;
    }

    elapsed_us_ += dt_us;

    switch (phase_) {
        case TrajectoryPhase::ACCEL:
            updateAccel(dt_us);
            break;
        case TrajectoryPhase::CRUISE:
            updateCruise(dt_us);
            break;
        case TrajectoryPhase::DECEL:
            updateDecel(dt_us);
            break;
        default:
            break;
    }

    // Build state
    state.phase = phase_;
    state.elapsed_us = elapsed_us_;
    state.position = current_position_ * direction_;  // Apply direction
    state.velocity = current_velocity_;
    state.remaining = (params_.distance - current_position_) * direction_;
    state.progress = (params_.distance > 0) ? 
        static_cast<float>(current_position_) / params_.distance : 1.0f;

    return state;
}

void TrapezoidalTrajectory::updateAccel(uint32_t dt_us)
{
    // v = v0 + a * t
    // delta_v = a * dt
    uint64_t delta_v = (static_cast<uint64_t>(params_.acceleration) * dt_us) / 1000000;
    current_velocity_ += static_cast<uint32_t>(delta_v);

    // Clamp to peak velocity
    if (current_velocity_ >= timing_.peak_velocity) {
        current_velocity_ = timing_.peak_velocity;
    }

    // Update position: delta_pos = v * dt (using average velocity)
    uint64_t delta_pos = (static_cast<uint64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(delta_pos);

    // Check phase transition
    if (elapsed_us_ >= timing_.accel_time_us) {
        if (timing_.is_triangle) {
            phase_ = TrajectoryPhase::DECEL;
        } else {
            phase_ = TrajectoryPhase::CRUISE;
        }
    }
}

void TrapezoidalTrajectory::updateCruise(uint32_t dt_us)
{
    // Constant velocity
    current_velocity_ = timing_.peak_velocity;

    // Update position
    uint64_t delta_pos = (static_cast<uint64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(delta_pos);

    // Check phase transition
    if (elapsed_us_ >= timing_.accel_time_us + timing_.cruise_time_us) {
        phase_ = TrajectoryPhase::DECEL;
    }
}

void TrapezoidalTrajectory::updateDecel(uint32_t dt_us)
{
    // v = v0 - d * t
    // delta_v = d * dt
    uint64_t delta_v = (static_cast<uint64_t>(params_.deceleration) * dt_us) / 1000000;
    
    if (delta_v < current_velocity_) {
        current_velocity_ -= static_cast<uint32_t>(delta_v);
    } else {
        current_velocity_ = params_.end_velocity;
    }

    // Update position
    uint64_t delta_pos = (static_cast<uint64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(delta_pos);

    // Check completion
    if (current_position_ >= params_.distance || 
        elapsed_us_ >= timing_.total_time_us) {
        current_position_ = params_.distance;
        current_velocity_ = params_.end_velocity;
        phase_ = TrajectoryPhase::COMPLETE;
    }
}

void TrapezoidalTrajectory::reset()
{
    phase_ = TrajectoryPhase::IDLE;
    elapsed_us_ = 0;
    current_position_ = 0;
    current_velocity_ = 0;
    planned_ = false;
    started_ = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────────────────────────────────────

TrajectoryPhase TrapezoidalTrajectory::getPhase() const
{
    return phase_;
}

const TrajectoryTiming& TrapezoidalTrajectory::getTiming() const
{
    return timing_;
}

bool TrapezoidalTrajectory::isComplete() const
{
    return phase_ == TrajectoryPhase::COMPLETE;
}

uint32_t TrapezoidalTrajectory::getCurrentVelocity() const
{
    return current_velocity_;
}

int32_t TrapezoidalTrajectory::getCurrentPosition() const
{
    return current_position_ * direction_;
}

uint32_t TrapezoidalTrajectory::getVelocityAt(uint32_t time_us) const
{
    if (!planned_ || time_us == 0) {
        return params_.start_velocity;
    }

    if (time_us >= timing_.total_time_us) {
        return params_.end_velocity;
    }

    if (time_us < timing_.accel_time_us) {
        // Accelerating
        return params_.start_velocity + 
            static_cast<uint32_t>((static_cast<uint64_t>(params_.acceleration) * time_us) / 1000000);
    }

    uint32_t cruise_end = timing_.accel_time_us + timing_.cruise_time_us;
    if (time_us < cruise_end) {
        // Cruising
        return timing_.peak_velocity;
    }

    // Decelerating
    uint32_t decel_elapsed = time_us - cruise_end;
    uint32_t decel_amount = static_cast<uint32_t>(
        (static_cast<uint64_t>(params_.deceleration) * decel_elapsed) / 1000000);
    
    if (decel_amount >= timing_.peak_velocity) {
        return params_.end_velocity;
    }
    return timing_.peak_velocity - decel_amount;
}

int32_t TrapezoidalTrajectory::getPositionAt(uint32_t time_us) const
{
    if (!planned_ || time_us == 0) {
        return 0;
    }

    if (time_us >= timing_.total_time_us) {
        return params_.distance * direction_;
    }

    // This is a simplified calculation
    // For accurate results, integrate velocity over time
    // For now, use linear interpolation based on phase

    if (time_us < timing_.accel_time_us) {
        // Accelerating: s = v0*t + 0.5*a*t²
        uint64_t t = time_us;
        uint64_t a = params_.acceleration;
        uint64_t v0 = params_.start_velocity;
        
        // s = v0 * t / 1e6 + 0.5 * a * t² / 1e12
        uint64_t pos = (v0 * t) / 1000000 + (a * t * t) / (2 * 1000000ULL * 1000000ULL);
        return static_cast<int32_t>(pos) * direction_;
    }

    uint32_t cruise_end = timing_.accel_time_us + timing_.cruise_time_us;
    if (time_us < cruise_end) {
        // Cruising: accel_distance + v * (t - accel_time)
        uint64_t cruise_time = time_us - timing_.accel_time_us;
        uint64_t cruise_pos = (static_cast<uint64_t>(timing_.peak_velocity) * cruise_time) / 1000000;
        return (timing_.accel_distance + static_cast<int32_t>(cruise_pos)) * direction_;
    }

    // Decelerating
    int32_t base_pos = timing_.accel_distance + timing_.cruise_distance;
    uint32_t decel_time = time_us - cruise_end;
    
    // s = v0*t - 0.5*d*t²
    uint64_t v0 = timing_.peak_velocity;
    uint64_t d = params_.deceleration;
    uint64_t t = decel_time;
    
    uint64_t decel_pos = (v0 * t) / 1000000;
    uint64_t decel_loss = (d * t * t) / (2 * 1000000ULL * 1000000ULL);
    
    return (base_pos + static_cast<int32_t>(decel_pos - decel_loss)) * direction_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility Functions
// ─────────────────────────────────────────────────────────────────────────────

const char* trajectoryPhaseToString(TrajectoryPhase phase)
{
    switch (phase) {
        case TrajectoryPhase::IDLE:     return "IDLE";
        case TrajectoryPhase::ACCEL:    return "ACCEL";
        case TrajectoryPhase::CRUISE:   return "CRUISE";
        case TrajectoryPhase::DECEL:    return "DECEL";
        case TrajectoryPhase::COMPLETE: return "COMPLETE";
        default:                        return "UNKNOWN";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// S-Curve Trajectory Implementation
// ─────────────────────────────────────────────────────────────────────────────

SCurveTrajectory::SCurveTrajectory()
    : params_{}
    , timing_{}
    , phase_(SCurvePhase::IDLE)
    , elapsed_us_(0)
    , phase_elapsed_us_(0)
    , current_position_(0)
    , current_velocity_(0)
    , current_acceleration_(0)
    , planned_(false)
    , started_(false)
    , direction_(1)
    , t_end_j1_(0)
    , t_end_a_(0)
    , t_end_j2_(0)
    , t_end_c_(0)
    , t_end_j3_(0)
    , t_end_d_(0)
{
}

bool SCurveTrajectory::plan(const SCurveParams& params)
{
    // Validate parameters
    if (params.max_velocity == 0 || params.max_acceleration == 0 || 
        params.max_jerk == 0) {
        return false;
    }

    params_ = params;

    // Determine direction
    if (params_.distance >= 0) {
        direction_ = 1;
    } else {
        direction_ = -1;
        params_.distance = -params_.distance;
    }

    // Compute the full or reduced profile
    // Time to reach max acceleration with max jerk: t_j = a_max / j_max
    uint64_t j = params_.max_jerk;
    uint64_t a = params_.max_acceleration;
    uint64_t v = params_.max_velocity;

    // Velocity gained during one jerk phase: delta_v = 0.5 * j * t_j²
    // = 0.5 * a_max² / j_max
    uint64_t v_jerk = (a * a) / (2 * j);

    // Velocity at end of accel phases (2 jerk phases + const accel)
    // If v_max can be reached with just the two jerk phases:
    if (2 * v_jerk >= v) {
        // Need to reduce profile - can't reach max acceleration
        computeReducedProfile();
    } else {
        // Full 7-segment profile possible
        computeFullProfile();
    }

    // Compute cumulative phase boundary times
    t_end_j1_ = timing_.t_jerk_accel;
    t_end_a_ = t_end_j1_ + timing_.t_const_accel;
    t_end_j2_ = t_end_a_ + timing_.t_jerk_accel;
    t_end_c_ = t_end_j2_ + timing_.t_cruise;
    t_end_j3_ = t_end_c_ + timing_.t_jerk_decel;
    t_end_d_ = t_end_j3_ + timing_.t_const_decel;
    // t_end_j4 = total_time

    planned_ = true;
    started_ = false;
    phase_ = SCurvePhase::IDLE;

    return true;
}

void SCurveTrajectory::computeFullProfile()
{
    uint64_t j = params_.max_jerk;
    uint64_t a = params_.max_acceleration;
    uint64_t v = params_.max_velocity;

    // Time for jerk phase (µs)
    timing_.t_jerk_accel = static_cast<uint32_t>((a * 1000000ULL) / j);
    timing_.t_jerk_decel = timing_.t_jerk_accel;

    // Velocity gained during jerk phases
    uint64_t v_jerk = (a * a) / (2 * j);

    // Velocity remaining for constant acceleration phase
    uint64_t v_const_accel = v - 2 * v_jerk;

    // Time for constant acceleration (µs)
    timing_.t_const_accel = static_cast<uint32_t>((v_const_accel * 1000000ULL) / a);
    timing_.t_const_decel = timing_.t_const_accel;

    // Distance covered during acceleration phases
    // d_j1 = (1/6) * j * t_j³  (during first jerk)
    // d_a  = a * t_a² / 2 + v_at_start_of_a * t_a
    // d_j2 = ... (similar integration)

    // Simplified: total accel distance
    uint64_t t_j = timing_.t_jerk_accel;
    uint64_t t_a = timing_.t_const_accel;

    // Distance during J1: d = (1/6)*j*t³  (in steps, converting µs)
    uint64_t d_j1 = (j * t_j * t_j * t_j) / (6ULL * 1000000ULL * 1000000ULL * 1000000ULL);

    // Velocity at end of J1
    uint64_t v_end_j1 = (j * t_j * t_j) / (2ULL * 1000000ULL * 1000000ULL);

    // Distance during A: d = v_end_j1 * t_a + 0.5 * a * t_a²
    uint64_t d_a = (v_end_j1 * t_a) / 1000000ULL + 
                   (a * t_a * t_a) / (2ULL * 1000000ULL * 1000000ULL);

    // Velocity at end of A
    uint64_t v_end_a = v_end_j1 + (a * t_a) / 1000000ULL;

    // Distance during J2 (accel decreasing from a to 0)
    uint64_t d_j2 = (v_end_a * t_j) / 1000000ULL + 
                    (a * t_j * t_j) / (2ULL * 1000000ULL * 1000000ULL) -
                    (j * t_j * t_j * t_j) / (6ULL * 1000000ULL * 1000000ULL * 1000000ULL);

    uint64_t total_accel_dist = d_j1 + d_a + d_j2;
    uint64_t total_decel_dist = total_accel_dist;  // Symmetric

    // Cruise distance
    int64_t cruise_dist = params_.distance - static_cast<int64_t>(total_accel_dist + total_decel_dist);

    if (cruise_dist < 0) {
        // Can't reach max velocity - need reduced profile
        computeReducedProfile();
        return;
    }

    // Cruise time (µs)
    timing_.t_cruise = static_cast<uint32_t>((static_cast<uint64_t>(cruise_dist) * 1000000ULL) / v);

    timing_.v_achieved = static_cast<uint32_t>(v);
    timing_.a_achieved = static_cast<uint32_t>(a);
    timing_.is_reduced = false;

    timing_.total_time_us = 2 * timing_.t_jerk_accel + timing_.t_const_accel +
                            timing_.t_cruise +
                            2 * timing_.t_jerk_decel + timing_.t_const_decel;
}

void SCurveTrajectory::computeReducedProfile()
{
    // For short moves, we need to reduce either:
    // 1. Peak velocity (no cruise phase)
    // 2. Peak acceleration (if can't even reach max accel)

    uint64_t j = params_.max_jerk;
    uint64_t a = params_.max_acceleration;
    uint64_t dist = static_cast<uint64_t>(params_.distance);

    // Minimum distance for full accel profile: 2 * (a³ / (3 * j²))
    // This is when const_accel time is 0 and we just have jerk phases

    // Simplified approach: binary search for achievable velocity
    uint32_t v_low = 0;
    uint32_t v_high = params_.max_velocity;
    uint32_t v_achieved = 0;

    for (int iter = 0; iter < 32; iter++) {
        uint32_t v_try = (v_low + v_high) / 2;
        if (v_try == 0) break;

        // Time for jerk phases
        uint64_t t_j = (a * 1000000ULL) / j;

        // Velocity from jerk phases
        uint64_t v_jerk = (a * a) / (2 * j);

        if (v_try <= 2 * v_jerk) {
            // Need shorter jerk phases
            // v = 2 * (0.5 * j * t²) = j * t²
            // t = sqrt(v / j)
            double t_j_s = std::sqrt(static_cast<double>(v_try) / j);
            t_j = static_cast<uint64_t>(t_j_s * 1000000.0);
            v_jerk = v_try / 2;
        }

        uint64_t v_const = (v_try > 2 * v_jerk) ? (v_try - 2 * v_jerk) : 0;
        uint64_t t_a = (a > 0 && v_const > 0) ? (v_const * 1000000ULL) / a : 0;

        // Compute distance with this velocity
        uint64_t d_j1 = (j * t_j * t_j * t_j) / (6ULL * 1000000ULL * 1000000ULL * 1000000ULL);
        uint64_t v_end_j1 = (j * t_j * t_j) / (2ULL * 1000000ULL * 1000000ULL);
        uint64_t d_a = (v_end_j1 * t_a) / 1000000ULL + 
                       (a * t_a * t_a) / (2ULL * 1000000ULL * 1000000ULL);
        uint64_t v_end_a = v_end_j1 + (a * t_a) / 1000000ULL;
        uint64_t d_j2 = (v_end_a * t_j) / 1000000ULL;

        uint64_t total_dist = 2 * (d_j1 + d_a + d_j2);  // Accel + decel

        if (total_dist <= dist) {
            v_achieved = v_try;
            v_low = v_try + 1;
        } else {
            v_high = v_try - 1;
        }
    }

    if (v_achieved == 0) {
        v_achieved = 100;  // Minimum velocity
    }

    // Now compute profile with achieved velocity
    uint64_t v_jerk = (a * a) / (2 * j);
    uint64_t t_j;

    if (v_achieved <= 2 * v_jerk) {
        double t_j_s = std::sqrt(static_cast<double>(v_achieved) / j);
        t_j = static_cast<uint64_t>(t_j_s * 1000000.0);
        timing_.t_const_accel = 0;
        timing_.t_const_decel = 0;
        timing_.a_achieved = static_cast<uint32_t>(j * t_j / 1000000);
    } else {
        t_j = (a * 1000000ULL) / j;
        uint64_t v_const = v_achieved - 2 * v_jerk;
        timing_.t_const_accel = static_cast<uint32_t>((v_const * 1000000ULL) / a);
        timing_.t_const_decel = timing_.t_const_accel;
        timing_.a_achieved = static_cast<uint32_t>(a);
    }

    timing_.t_jerk_accel = static_cast<uint32_t>(t_j);
    timing_.t_jerk_decel = timing_.t_jerk_accel;
    timing_.t_cruise = 0;  // No cruise for reduced profile
    timing_.v_achieved = v_achieved;
    timing_.is_reduced = true;

    timing_.total_time_us = 2 * timing_.t_jerk_accel + timing_.t_const_accel +
                            2 * timing_.t_jerk_decel + timing_.t_const_decel;
}

void SCurveTrajectory::start()
{
    if (!planned_) return;

    elapsed_us_ = 0;
    phase_elapsed_us_ = 0;
    current_position_ = 0;
    current_velocity_ = 0;
    current_acceleration_ = 0;
    started_ = true;
    phase_ = SCurvePhase::JERK_ACCEL_RISE;
}

SCurveState SCurveTrajectory::update(uint32_t dt_us)
{
    SCurveState state = {
        .phase = phase_,
        .elapsed_us = elapsed_us_,
        .position = current_position_ * direction_,
        .velocity = current_velocity_,
        .acceleration = current_acceleration_,
        .progress = 0.0f
    };

    if (!started_ || phase_ == SCurvePhase::IDLE || phase_ == SCurvePhase::COMPLETE) {
        return state;
    }

    elapsed_us_ += dt_us;
    phase_elapsed_us_ += dt_us;

    // Check for phase transition
    checkPhaseTransition();

    // Update based on current phase
    int32_t jerk = static_cast<int32_t>(params_.max_jerk);

    switch (phase_) {
        case SCurvePhase::JERK_ACCEL_RISE:
            updateJerkUp(dt_us, jerk);
            break;
        case SCurvePhase::CONST_ACCEL:
            updateConstAccel(dt_us);
            break;
        case SCurvePhase::JERK_ACCEL_FALL:
            updateJerkDown(dt_us, jerk);
            break;
        case SCurvePhase::CRUISE:
            updateCruise(dt_us);
            break;
        case SCurvePhase::JERK_DECEL_RISE:
            updateJerkDown(dt_us, jerk);  // Negative jerk
            break;
        case SCurvePhase::CONST_DECEL:
            updateConstDecel(dt_us);
            break;
        case SCurvePhase::JERK_DECEL_FALL:
            updateJerkUp(dt_us, jerk);  // Jerk reducing decel magnitude
            break;
        default:
            break;
    }

    // Compute progress
    state.phase = phase_;
    state.elapsed_us = elapsed_us_;
    state.position = current_position_ * direction_;
    state.velocity = current_velocity_;
    state.acceleration = current_acceleration_;
    state.progress = (timing_.total_time_us > 0) ?
        static_cast<float>(elapsed_us_) / timing_.total_time_us : 1.0f;

    if (state.progress > 1.0f) state.progress = 1.0f;

    return state;
}

void SCurveTrajectory::checkPhaseTransition()
{
    switch (phase_) {
        case SCurvePhase::JERK_ACCEL_RISE:
            if (elapsed_us_ >= t_end_j1_) {
                phase_ = (timing_.t_const_accel > 0) ? 
                         SCurvePhase::CONST_ACCEL : SCurvePhase::JERK_ACCEL_FALL;
                phase_elapsed_us_ = 0;
                current_acceleration_ = static_cast<int32_t>(timing_.a_achieved);
            }
            break;
        case SCurvePhase::CONST_ACCEL:
            if (elapsed_us_ >= t_end_a_) {
                phase_ = SCurvePhase::JERK_ACCEL_FALL;
                phase_elapsed_us_ = 0;
            }
            break;
        case SCurvePhase::JERK_ACCEL_FALL:
            if (elapsed_us_ >= t_end_j2_) {
                phase_ = (timing_.t_cruise > 0) ? 
                         SCurvePhase::CRUISE : SCurvePhase::JERK_DECEL_RISE;
                phase_elapsed_us_ = 0;
                current_acceleration_ = 0;
                current_velocity_ = timing_.v_achieved;
            }
            break;
        case SCurvePhase::CRUISE:
            if (elapsed_us_ >= t_end_c_) {
                phase_ = SCurvePhase::JERK_DECEL_RISE;
                phase_elapsed_us_ = 0;
            }
            break;
        case SCurvePhase::JERK_DECEL_RISE:
            if (elapsed_us_ >= t_end_j3_) {
                phase_ = (timing_.t_const_decel > 0) ?
                         SCurvePhase::CONST_DECEL : SCurvePhase::JERK_DECEL_FALL;
                phase_elapsed_us_ = 0;
                current_acceleration_ = -static_cast<int32_t>(timing_.a_achieved);
            }
            break;
        case SCurvePhase::CONST_DECEL:
            if (elapsed_us_ >= t_end_d_) {
                phase_ = SCurvePhase::JERK_DECEL_FALL;
                phase_elapsed_us_ = 0;
            }
            break;
        case SCurvePhase::JERK_DECEL_FALL:
            if (elapsed_us_ >= timing_.total_time_us) {
                phase_ = SCurvePhase::COMPLETE;
                current_velocity_ = 0;
                current_acceleration_ = 0;
                current_position_ = params_.distance;
            }
            break;
        default:
            break;
    }
}

void SCurveTrajectory::updateJerkUp(uint32_t dt_us, int32_t jerk)
{
    // Acceleration increases
    int64_t da = (static_cast<int64_t>(jerk) * dt_us) / 1000000;
    current_acceleration_ += static_cast<int32_t>(da);

    // Velocity changes
    int64_t dv = (static_cast<int64_t>(current_acceleration_) * dt_us) / 1000000;
    if (dv < 0 && static_cast<uint32_t>(-dv) > current_velocity_) {
        current_velocity_ = 0;
    } else {
        current_velocity_ = static_cast<uint32_t>(static_cast<int64_t>(current_velocity_) + dv);
    }

    // Position changes
    int64_t dp = (static_cast<int64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(dp);
}

void SCurveTrajectory::updateJerkDown(uint32_t dt_us, int32_t jerk)
{
    // Acceleration decreases
    int64_t da = (static_cast<int64_t>(jerk) * dt_us) / 1000000;
    current_acceleration_ -= static_cast<int32_t>(da);

    // Velocity changes
    int64_t dv = (static_cast<int64_t>(current_acceleration_) * dt_us) / 1000000;
    if (dv < 0 && static_cast<uint32_t>(-dv) > current_velocity_) {
        current_velocity_ = 0;
    } else {
        current_velocity_ = static_cast<uint32_t>(static_cast<int64_t>(current_velocity_) + dv);
    }

    // Position changes
    int64_t dp = (static_cast<int64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(dp);
}

void SCurveTrajectory::updateConstAccel(uint32_t dt_us)
{
    // Constant acceleration
    int64_t dv = (static_cast<int64_t>(current_acceleration_) * dt_us) / 1000000;
    current_velocity_ = static_cast<uint32_t>(static_cast<int64_t>(current_velocity_) + dv);

    // Position
    int64_t dp = (static_cast<int64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(dp);
}

void SCurveTrajectory::updateCruise(uint32_t dt_us)
{
    // Constant velocity
    int64_t dp = (static_cast<int64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(dp);
}

void SCurveTrajectory::updateConstDecel(uint32_t dt_us)
{
    // Constant deceleration (negative acceleration)
    int64_t dv = (static_cast<int64_t>(current_acceleration_) * dt_us) / 1000000;
    if (dv < 0 && static_cast<uint32_t>(-dv) > current_velocity_) {
        current_velocity_ = 0;
    } else {
        current_velocity_ = static_cast<uint32_t>(static_cast<int64_t>(current_velocity_) + dv);
    }

    // Position
    int64_t dp = (static_cast<int64_t>(current_velocity_) * dt_us) / 1000000;
    current_position_ += static_cast<int32_t>(dp);
}

void SCurveTrajectory::reset()
{
    phase_ = SCurvePhase::IDLE;
    elapsed_us_ = 0;
    phase_elapsed_us_ = 0;
    current_position_ = 0;
    current_velocity_ = 0;
    current_acceleration_ = 0;
    started_ = false;
}

SCurvePhase SCurveTrajectory::getPhase() const
{
    return phase_;
}

const SCurveTiming& SCurveTrajectory::getTiming() const
{
    return timing_;
}

bool SCurveTrajectory::isComplete() const
{
    return phase_ == SCurvePhase::COMPLETE;
}

uint32_t SCurveTrajectory::getCurrentVelocity() const
{
    return current_velocity_;
}

int32_t SCurveTrajectory::getCurrentPosition() const
{
    return current_position_ * direction_;
}

int32_t SCurveTrajectory::getCurrentAcceleration() const
{
    return current_acceleration_;
}

const char* sCurvePhaseToString(SCurvePhase phase)
{
    switch (phase) {
        case SCurvePhase::IDLE:             return "IDLE";
        case SCurvePhase::JERK_ACCEL_RISE:  return "JERK_ACCEL_RISE";
        case SCurvePhase::CONST_ACCEL:      return "CONST_ACCEL";
        case SCurvePhase::JERK_ACCEL_FALL:  return "JERK_ACCEL_FALL";
        case SCurvePhase::CRUISE:           return "CRUISE";
        case SCurvePhase::JERK_DECEL_RISE:  return "JERK_DECEL_RISE";
        case SCurvePhase::CONST_DECEL:      return "CONST_DECEL";
        case SCurvePhase::JERK_DECEL_FALL:  return "JERK_DECEL_FALL";
        case SCurvePhase::COMPLETE:         return "COMPLETE";
        default:                            return "UNKNOWN";
    }
}

}  // namespace usd
