/**
 * @file trajectory.h
 * @brief Trajectory generation for motion profiles
 *
 * Generates velocity profiles for smooth acceleration/deceleration.
 * Supports trapezoidal and S-curve (jerk-limited) profiles.
 *
 * @version 0.1.0
 * @date 2025-05-27
 */

#ifndef USD_TRAJECTORY_H
#define USD_TRAJECTORY_H

#include <cstdint>

namespace usd {

/**
 * @brief Trajectory segment type for trapezoidal profile
 */
enum class TrajectoryPhase : uint8_t {
    IDLE = 0,           ///< No active trajectory
    ACCEL = 1,          ///< Accelerating
    CRUISE = 2,         ///< Constant velocity
    DECEL = 3,          ///< Decelerating
    COMPLETE = 4        ///< Trajectory finished
};

/**
 * @brief Trajectory parameters for a move
 */
struct TrajectoryParams {
    int32_t distance;           ///< Total distance to travel (steps)
    uint32_t max_velocity;      ///< Maximum velocity (steps/sec)
    uint32_t acceleration;      ///< Acceleration rate (steps/sec²)
    uint32_t deceleration;      ///< Deceleration rate (steps/sec², 0 = use accel)
    uint32_t start_velocity;    ///< Starting velocity (typically 0)
    uint32_t end_velocity;      ///< Ending velocity (typically 0)
};

/**
 * @brief Computed trajectory timing
 */
struct TrajectoryTiming {
    uint32_t accel_time_us;     ///< Time spent accelerating (µs)
    uint32_t cruise_time_us;    ///< Time at cruise velocity (µs)
    uint32_t decel_time_us;     ///< Time spent decelerating (µs)
    uint32_t total_time_us;     ///< Total move time (µs)
    
    int32_t accel_distance;     ///< Distance covered during accel (steps)
    int32_t cruise_distance;    ///< Distance at cruise velocity (steps)
    int32_t decel_distance;     ///< Distance covered during decel (steps)
    
    uint32_t peak_velocity;     ///< Actual peak velocity reached (may be < max)
    bool is_triangle;           ///< True if no cruise phase (triangular profile)
};

/**
 * @brief Current trajectory state during execution
 */
struct TrajectoryState {
    TrajectoryPhase phase;      ///< Current phase
    uint32_t elapsed_us;        ///< Time since trajectory start
    int32_t position;           ///< Current position in trajectory
    uint32_t velocity;          ///< Current velocity
    int32_t remaining;          ///< Distance remaining
    float progress;             ///< 0.0 to 1.0 completion
};

/**
 * @brief Trapezoidal trajectory interpolator
 *
 * Generates velocity profiles with linear acceleration/deceleration.
 * This is the simplest and most common motion profile.
 *
 * Velocity profile:
 * ```
 *        ___________
 *       /           \
 *      /             \
 *     /               \
 * ___/                 \___
 *    accel  cruise  decel
 * ```
 *
 * For short moves where max velocity can't be reached, the profile
 * becomes triangular (no cruise phase):
 * ```
 *         /\
 *        /  \
 *       /    \
 * _____/      \____
 *     accel  decel
 * ```
 *
 * Thread Safety:
 * - plan() should be called before motion starts
 * - update() should be called from motion task at regular intervals
 *
 * Usage:
 * @code
 *   TrapezoidalTrajectory traj;
 *
 *   TrajectoryParams params = {
 *       .distance = 10000,
 *       .max_velocity = 50000,
 *       .acceleration = 100000,
 *       .deceleration = 100000
 *   };
 *
 *   traj.plan(params);
 *   traj.start();
 *
 *   while (traj.getPhase() != TrajectoryPhase::COMPLETE) {
 *       TrajectoryState state = traj.update(dt_us);
 *       stepper.setFrequency(state.velocity);
 *   }
 * @endcode
 */
class TrapezoidalTrajectory {
public:
    /**
     * @brief Default constructor
     */
    TrapezoidalTrajectory();

    /**
     * @brief Plan a trajectory with given parameters
     *
     * Computes timing and distances for each phase. Must be called
     * before start().
     *
     * @param params Trajectory parameters
     * @return true if trajectory is valid, false if parameters invalid
     */
    bool plan(const TrajectoryParams& params);

    /**
     * @brief Start trajectory execution
     *
     * Resets internal state and begins from the start.
     */
    void start();

    /**
     * @brief Update trajectory state
     *
     * Call this at regular intervals from the motion task.
     * Returns the current state including velocity to apply.
     *
     * @param dt_us Time since last update in microseconds
     * @return Current trajectory state
     */
    TrajectoryState update(uint32_t dt_us);

    /**
     * @brief Reset trajectory to idle
     */
    void reset();

    /**
     * @brief Get current phase
     */
    TrajectoryPhase getPhase() const;

    /**
     * @brief Get computed timing information
     */
    const TrajectoryTiming& getTiming() const;

    /**
     * @brief Check if trajectory is complete
     */
    bool isComplete() const;

    /**
     * @brief Get current velocity
     */
    uint32_t getCurrentVelocity() const;

    /**
     * @brief Get current position in trajectory
     */
    int32_t getCurrentPosition() const;

    /**
     * @brief Get velocity at a specific time point
     *
     * Useful for look-ahead and synchronization.
     *
     * @param time_us Time from trajectory start
     * @return Velocity at that time point
     */
    uint32_t getVelocityAt(uint32_t time_us) const;

    /**
     * @brief Get position at a specific time point
     *
     * @param time_us Time from trajectory start
     * @return Position at that time point
     */
    int32_t getPositionAt(uint32_t time_us) const;

private:
    TrajectoryParams params_;
    TrajectoryTiming timing_;
    TrajectoryPhase phase_;

    uint32_t elapsed_us_;
    int32_t current_position_;
    uint32_t current_velocity_;

    bool planned_;
    bool started_;
    int8_t direction_;  // 1 for positive, -1 for negative

    /**
     * @brief Compute timing for standard trapezoidal profile
     */
    void computeTrapezoidal();

    /**
     * @brief Compute timing for triangular profile (short move)
     */
    void computeTriangular();

    /**
     * @brief Update velocity during acceleration phase
     */
    void updateAccel(uint32_t dt_us);

    /**
     * @brief Update velocity during cruise phase
     */
    void updateCruise(uint32_t dt_us);

    /**
     * @brief Update velocity during deceleration phase
     */
    void updateDecel(uint32_t dt_us);
};

/**
 * @brief Get string representation of trajectory phase
 */
const char* trajectoryPhaseToString(TrajectoryPhase phase);

// ─────────────────────────────────────────────────────────────────────────────
// S-Curve (Jerk-Limited) Trajectory
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief S-Curve phase for 7-segment profile
 *
 * An S-curve profile has 7 phases:
 *   J+  → Jerk increasing acceleration
 *   A   → Constant acceleration
 *   J-  → Jerk decreasing acceleration (to zero)
 *   C   → Cruise at constant velocity
 *   J-  → Jerk decreasing velocity (start decel)
 *   D   → Constant deceleration
 *   J+  → Jerk returning to zero
 */
enum class SCurvePhase : uint8_t {
    IDLE = 0,
    JERK_ACCEL_RISE = 1,    ///< J1: Increasing acceleration
    CONST_ACCEL = 2,        ///< A:  Constant max acceleration
    JERK_ACCEL_FALL = 3,    ///< J2: Decreasing acceleration
    CRUISE = 4,             ///< C:  Constant velocity
    JERK_DECEL_RISE = 5,    ///< J3: Increasing deceleration
    CONST_DECEL = 6,        ///< D:  Constant max deceleration
    JERK_DECEL_FALL = 7,    ///< J4: Decreasing deceleration
    COMPLETE = 8
};

/**
 * @brief S-Curve trajectory parameters
 */
struct SCurveParams {
    int32_t distance;           ///< Total distance (steps)
    uint32_t max_velocity;      ///< Maximum velocity (steps/sec)
    uint32_t max_acceleration;  ///< Maximum acceleration (steps/sec²)
    uint32_t max_jerk;          ///< Maximum jerk (steps/sec³)
};

/**
 * @brief S-Curve timing for each of the 7 phases
 */
struct SCurveTiming {
    uint32_t t_jerk_accel;      ///< Time for jerk phases in accel (µs)
    uint32_t t_const_accel;     ///< Time for constant accel phase (µs)
    uint32_t t_cruise;          ///< Time at cruise velocity (µs)
    uint32_t t_jerk_decel;      ///< Time for jerk phases in decel (µs)
    uint32_t t_const_decel;     ///< Time for constant decel phase (µs)
    uint32_t total_time_us;     ///< Total move time (µs)

    uint32_t v_achieved;        ///< Actual peak velocity achieved
    uint32_t a_achieved;        ///< Actual peak acceleration achieved
    bool is_reduced;            ///< True if profile was reduced (short move)
};

/**
 * @brief S-Curve trajectory state
 */
struct SCurveState {
    SCurvePhase phase;
    uint32_t elapsed_us;
    int32_t position;
    uint32_t velocity;
    int32_t acceleration;       ///< Current acceleration (can be negative)
    float progress;
};

/**
 * @brief S-Curve (jerk-limited) trajectory interpolator
 *
 * Generates smooth velocity profiles with limited jerk, resulting in
 * smoother motion with reduced mechanical stress and vibration.
 *
 * 7-Phase velocity profile:
 * ```
 *           ___________
 *          /           \
 *         /             \         ← S-curve (smooth corners)
 *        /               \
 *   ____/                 \____
 *   J+ A J-    C    J- D J+
 *    (accel)  (cruise) (decel)
 * ```
 *
 * Compared to trapezoidal:
 * - Trapezoidal: instant acceleration changes (infinite jerk)
 * - S-Curve: smooth acceleration changes (limited jerk)
 *
 * Benefits:
 * - Reduced vibration and mechanical stress
 * - Lower acoustic noise
 * - Better for high-precision applications
 *
 * Trade-offs:
 * - Slightly longer move times for same distance
 * - More complex computation
 *
 * Usage:
 * @code
 *   SCurveTrajectory traj;
 *
 *   SCurveParams params = {
 *       .distance = 10000,
 *       .max_velocity = 50000,
 *       .max_acceleration = 100000,
 *       .max_jerk = 500000
 *   };
 *
 *   traj.plan(params);
 *   traj.start();
 *
 *   while (!traj.isComplete()) {
 *       SCurveState state = traj.update(dt_us);
 *       stepper.setFrequency(state.velocity);
 *   }
 * @endcode
 */
class SCurveTrajectory {
public:
    SCurveTrajectory();

    /**
     * @brief Plan an S-curve trajectory
     *
     * Computes the 7-segment timing. May reduce peak velocity and
     * acceleration for short moves.
     *
     * @param params S-curve parameters
     * @return true if valid, false if parameters invalid
     */
    bool plan(const SCurveParams& params);

    /**
     * @brief Start trajectory execution
     */
    void start();

    /**
     * @brief Update trajectory state
     *
     * @param dt_us Time since last update
     * @return Current state
     */
    SCurveState update(uint32_t dt_us);

    /**
     * @brief Reset to idle
     */
    void reset();

    /**
     * @brief Get current phase
     */
    SCurvePhase getPhase() const;

    /**
     * @brief Get timing information
     */
    const SCurveTiming& getTiming() const;

    /**
     * @brief Check if complete
     */
    bool isComplete() const;

    /**
     * @brief Get current velocity
     */
    uint32_t getCurrentVelocity() const;

    /**
     * @brief Get current position
     */
    int32_t getCurrentPosition() const;

    /**
     * @brief Get current acceleration
     */
    int32_t getCurrentAcceleration() const;

private:
    SCurveParams params_;
    SCurveTiming timing_;
    SCurvePhase phase_;

    uint32_t elapsed_us_;
    uint32_t phase_elapsed_us_;
    int32_t current_position_;
    uint32_t current_velocity_;
    int32_t current_acceleration_;

    bool planned_;
    bool started_;
    int8_t direction_;

    // Phase boundary times (cumulative)
    uint32_t t_end_j1_;     ///< End of JERK_ACCEL_RISE
    uint32_t t_end_a_;      ///< End of CONST_ACCEL
    uint32_t t_end_j2_;     ///< End of JERK_ACCEL_FALL
    uint32_t t_end_c_;      ///< End of CRUISE
    uint32_t t_end_j3_;     ///< End of JERK_DECEL_RISE
    uint32_t t_end_d_;      ///< End of CONST_DECEL
    // t_end_j4 = total_time

    /**
     * @brief Compute full 7-segment profile
     */
    void computeFullProfile();

    /**
     * @brief Compute reduced profile for short moves
     */
    void computeReducedProfile();

    /**
     * @brief Advance phase if time elapsed
     */
    void checkPhaseTransition();

    /**
     * @brief Update during jerk phase (accel increasing)
     */
    void updateJerkUp(uint32_t dt_us, int32_t jerk);

    /**
     * @brief Update during jerk phase (accel decreasing)
     */
    void updateJerkDown(uint32_t dt_us, int32_t jerk);

    /**
     * @brief Update during constant acceleration phase
     */
    void updateConstAccel(uint32_t dt_us);

    /**
     * @brief Update during cruise phase
     */
    void updateCruise(uint32_t dt_us);

    /**
     * @brief Update during constant deceleration phase
     */
    void updateConstDecel(uint32_t dt_us);
};

/**
 * @brief Get string representation of S-curve phase
 */
const char* sCurvePhaseToString(SCurvePhase phase);

}  // namespace usd

#endif  // USD_TRAJECTORY_H
