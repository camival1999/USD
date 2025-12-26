/**
 * @file test_trajectory.cpp
 * @brief Unit tests for TrapezoidalTrajectory
 *
 * Tests trajectory planning and execution
 */

#include <unity.h>
#include "trajectory.h"

using namespace usd;

// ─────────────────────────────────────────────────────────────────────────────
// Planning Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_trajectory_plan_valid(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 10000,
        .max_velocity = 10000,
        .acceleration = 50000,
        .deceleration = 50000,
        .start_velocity = 0,
        .end_velocity = 0
    };
    
    bool result = traj.plan(params);
    TEST_ASSERT_TRUE(result);
}

void test_trajectory_plan_zero_velocity_fails(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 10000,
        .max_velocity = 0,  // Invalid
        .acceleration = 50000
    };
    
    bool result = traj.plan(params);
    TEST_ASSERT_FALSE(result);
}

void test_trajectory_plan_zero_acceleration_fails(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 10000,
        .max_velocity = 10000,
        .acceleration = 0  // Invalid
    };
    
    bool result = traj.plan(params);
    TEST_ASSERT_FALSE(result);
}

void test_trajectory_starts_idle(void)
{
    TrapezoidalTrajectory traj;
    
    TEST_ASSERT_EQUAL(TrajectoryPhase::IDLE, traj.getPhase());
}

// ─────────────────────────────────────────────────────────────────────────────
// Timing Calculation Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_trajectory_timing_trapezoidal(void)
{
    TrapezoidalTrajectory traj;
    
    // Long move - should be trapezoidal
    TrajectoryParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .acceleration = 50000,
        .deceleration = 50000,
        .start_velocity = 0,
        .end_velocity = 0
    };
    
    traj.plan(params);
    const TrajectoryTiming& timing = traj.getTiming();
    
    TEST_ASSERT_FALSE(timing.is_triangle);
    TEST_ASSERT_EQUAL_UINT32(10000, timing.peak_velocity);
    TEST_ASSERT_TRUE(timing.cruise_distance > 0);
    TEST_ASSERT_TRUE(timing.cruise_time_us > 0);
}

void test_trajectory_timing_triangular(void)
{
    TrapezoidalTrajectory traj;
    
    // Short move - should be triangular (can't reach max velocity)
    TrajectoryParams params = {
        .distance = 100,  // Very short
        .max_velocity = 10000,
        .acceleration = 50000,
        .deceleration = 50000,
        .start_velocity = 0,
        .end_velocity = 0
    };
    
    traj.plan(params);
    const TrajectoryTiming& timing = traj.getTiming();
    
    TEST_ASSERT_TRUE(timing.is_triangle);
    TEST_ASSERT_TRUE(timing.peak_velocity < params.max_velocity);
    TEST_ASSERT_EQUAL_INT32(0, timing.cruise_distance);
    TEST_ASSERT_EQUAL_UINT32(0, timing.cruise_time_us);
}

void test_trajectory_distance_sums_correctly(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 50000,
        .max_velocity = 10000,
        .acceleration = 25000,
        .deceleration = 25000,
        .start_velocity = 0,
        .end_velocity = 0
    };
    
    traj.plan(params);
    const TrajectoryTiming& timing = traj.getTiming();
    
    int32_t total = timing.accel_distance + timing.cruise_distance + timing.decel_distance;
    // Allow small rounding error
    TEST_ASSERT_INT32_WITHIN(10, params.distance, total);
}

// ─────────────────────────────────────────────────────────────────────────────
// Execution Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_trajectory_start_changes_phase(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 10000,
        .max_velocity = 10000,
        .acceleration = 50000
    };
    
    traj.plan(params);
    traj.start();
    
    TEST_ASSERT_EQUAL(TrajectoryPhase::ACCEL, traj.getPhase());
}

void test_trajectory_update_increases_velocity(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .acceleration = 50000,
        .deceleration = 50000
    };
    
    traj.plan(params);
    traj.start();
    
    uint32_t v1 = traj.getCurrentVelocity();
    traj.update(1000);  // 1ms
    uint32_t v2 = traj.getCurrentVelocity();
    
    TEST_ASSERT_TRUE(v2 > v1);
}

void test_trajectory_update_increases_position(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .acceleration = 50000
    };
    
    traj.plan(params);
    traj.start();
    
    int32_t p1 = traj.getCurrentPosition();
    traj.update(10000);  // 10ms
    int32_t p2 = traj.getCurrentPosition();
    
    TEST_ASSERT_TRUE(p2 > p1);
}

void test_trajectory_reaches_complete(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 1000,
        .max_velocity = 10000,
        .acceleration = 100000,
        .deceleration = 100000
    };
    
    traj.plan(params);
    traj.start();
    
    // Run trajectory to completion
    int iterations = 0;
    while (traj.getPhase() != TrajectoryPhase::COMPLETE && iterations < 10000) {
        traj.update(100);  // 100µs steps
        iterations++;
    }
    
    TEST_ASSERT_EQUAL(TrajectoryPhase::COMPLETE, traj.getPhase());
    TEST_ASSERT_TRUE(traj.isComplete());
}

void test_trajectory_negative_distance(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = -5000,  // Negative
        .max_velocity = 10000,
        .acceleration = 50000
    };
    
    bool result = traj.plan(params);
    TEST_ASSERT_TRUE(result);
    
    traj.start();
    traj.update(10000);
    
    // Position should be negative
    TEST_ASSERT_TRUE(traj.getCurrentPosition() <= 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// State Query Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_trajectory_get_velocity_at(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .acceleration = 50000,
        .deceleration = 50000
    };
    
    traj.plan(params);
    
    // At start
    uint32_t v0 = traj.getVelocityAt(0);
    TEST_ASSERT_EQUAL_UINT32(0, v0);
    
    // During accel
    uint32_t v_accel = traj.getVelocityAt(100000);  // 100ms
    TEST_ASSERT_TRUE(v_accel > 0);
    TEST_ASSERT_TRUE(v_accel <= params.max_velocity);
}

void test_trajectory_reset(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 10000,
        .max_velocity = 10000,
        .acceleration = 50000
    };
    
    traj.plan(params);
    traj.start();
    traj.update(1000);
    
    traj.reset();
    
    TEST_ASSERT_EQUAL(TrajectoryPhase::IDLE, traj.getPhase());
    TEST_ASSERT_EQUAL_INT32(0, traj.getCurrentPosition());
    TEST_ASSERT_EQUAL_UINT32(0, traj.getCurrentVelocity());
}

// ─────────────────────────────────────────────────────────────────────────────
// Phase String Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_trajectory_phase_to_string(void)
{
    TEST_ASSERT_EQUAL_STRING("IDLE", trajectoryPhaseToString(TrajectoryPhase::IDLE));
    TEST_ASSERT_EQUAL_STRING("ACCEL", trajectoryPhaseToString(TrajectoryPhase::ACCEL));
    TEST_ASSERT_EQUAL_STRING("CRUISE", trajectoryPhaseToString(TrajectoryPhase::CRUISE));
    TEST_ASSERT_EQUAL_STRING("DECEL", trajectoryPhaseToString(TrajectoryPhase::DECEL));
    TEST_ASSERT_EQUAL_STRING("COMPLETE", trajectoryPhaseToString(TrajectoryPhase::COMPLETE));
}

// ─────────────────────────────────────────────────────────────────────────────
// State Struct Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_trajectory_state_progress(void)
{
    TrapezoidalTrajectory traj;
    
    TrajectoryParams params = {
        .distance = 1000,
        .max_velocity = 10000,
        .acceleration = 100000,
        .deceleration = 100000
    };
    
    traj.plan(params);
    traj.start();
    
    // Initial progress should be near 0
    TrajectoryState state = traj.update(0);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, state.progress);
    
    // Run to completion
    while (!traj.isComplete()) {
        state = traj.update(100);
    }
    
    // Final progress should be 1.0
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, state.progress);
}

// ─────────────────────────────────────────────────────────────────────────────
// S-Curve Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_scurve_plan_valid(void)
{
    SCurveTrajectory traj;
    
    SCurveParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .max_acceleration = 50000,
        .max_jerk = 200000
    };
    
    bool result = traj.plan(params);
    TEST_ASSERT_TRUE(result);
}

void test_scurve_plan_zero_jerk_fails(void)
{
    SCurveTrajectory traj;
    
    SCurveParams params = {
        .distance = 10000,
        .max_velocity = 10000,
        .max_acceleration = 50000,
        .max_jerk = 0  // Invalid
    };
    
    bool result = traj.plan(params);
    TEST_ASSERT_FALSE(result);
}

void test_scurve_starts_idle(void)
{
    SCurveTrajectory traj;
    TEST_ASSERT_EQUAL(SCurvePhase::IDLE, traj.getPhase());
}

void test_scurve_start_changes_phase(void)
{
    SCurveTrajectory traj;
    
    SCurveParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .max_acceleration = 50000,
        .max_jerk = 200000
    };
    
    traj.plan(params);
    traj.start();
    
    TEST_ASSERT_EQUAL(SCurvePhase::JERK_ACCEL_RISE, traj.getPhase());
}

void test_scurve_velocity_smooth_increase(void)
{
    SCurveTrajectory traj;
    
    SCurveParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .max_acceleration = 50000,
        .max_jerk = 200000
    };
    
    traj.plan(params);
    traj.start();
    
    uint32_t prev_v = 0;
    for (int i = 0; i < 10; i++) {
        traj.update(1000);  // 1ms
        uint32_t v = traj.getCurrentVelocity();
        TEST_ASSERT_TRUE(v >= prev_v);  // Velocity should not decrease in accel
        prev_v = v;
    }
}

void test_scurve_reaches_complete(void)
{
    SCurveTrajectory traj;
    
    SCurveParams params = {
        .distance = 5000,
        .max_velocity = 10000,
        .max_acceleration = 100000,
        .max_jerk = 500000
    };
    
    traj.plan(params);
    traj.start();
    
    int iterations = 0;
    while (traj.getPhase() != SCurvePhase::COMPLETE && iterations < 50000) {
        traj.update(100);
        iterations++;
    }
    
    TEST_ASSERT_EQUAL(SCurvePhase::COMPLETE, traj.getPhase());
    TEST_ASSERT_TRUE(traj.isComplete());
}

void test_scurve_reduced_profile_short_move(void)
{
    SCurveTrajectory traj;
    
    // Very short move - should trigger reduced profile
    SCurveParams params = {
        .distance = 100,
        .max_velocity = 10000,
        .max_acceleration = 50000,
        .max_jerk = 200000
    };
    
    traj.plan(params);
    const SCurveTiming& timing = traj.getTiming();
    
    TEST_ASSERT_TRUE(timing.is_reduced);
    TEST_ASSERT_TRUE(timing.v_achieved < params.max_velocity);
}

void test_scurve_get_acceleration(void)
{
    SCurveTrajectory traj;
    
    SCurveParams params = {
        .distance = 100000,
        .max_velocity = 10000,
        .max_acceleration = 50000,
        .max_jerk = 200000
    };
    
    traj.plan(params);
    traj.start();
    
    // During first jerk phase, acceleration should increase
    traj.update(1000);
    int32_t a1 = traj.getCurrentAcceleration();
    traj.update(1000);
    int32_t a2 = traj.getCurrentAcceleration();
    
    TEST_ASSERT_TRUE(a2 > a1);  // Acceleration increasing
}

void test_scurve_phase_to_string(void)
{
    TEST_ASSERT_EQUAL_STRING("IDLE", sCurvePhaseToString(SCurvePhase::IDLE));
    TEST_ASSERT_EQUAL_STRING("JERK_ACCEL_RISE", sCurvePhaseToString(SCurvePhase::JERK_ACCEL_RISE));
    TEST_ASSERT_EQUAL_STRING("CONST_ACCEL", sCurvePhaseToString(SCurvePhase::CONST_ACCEL));
    TEST_ASSERT_EQUAL_STRING("CRUISE", sCurvePhaseToString(SCurvePhase::CRUISE));
    TEST_ASSERT_EQUAL_STRING("COMPLETE", sCurvePhaseToString(SCurvePhase::COMPLETE));
}

void test_scurve_reset(void)
{
    SCurveTrajectory traj;
    
    SCurveParams params = {
        .distance = 10000,
        .max_velocity = 10000,
        .max_acceleration = 50000,
        .max_jerk = 200000
    };
    
    traj.plan(params);
    traj.start();
    traj.update(5000);
    
    traj.reset();
    
    TEST_ASSERT_EQUAL(SCurvePhase::IDLE, traj.getPhase());
    TEST_ASSERT_EQUAL_INT32(0, traj.getCurrentPosition());
    TEST_ASSERT_EQUAL_UINT32(0, traj.getCurrentVelocity());
    TEST_ASSERT_EQUAL_INT32(0, traj.getCurrentAcceleration());
}

// ─────────────────────────────────────────────────────────────────────────────
// Test Runner
// ─────────────────────────────────────────────────────────────────────────────

void setUp(void) {}
void tearDown(void) {}

int main(int, char**)
{
    UNITY_BEGIN();

    // Trapezoidal planning tests
    RUN_TEST(test_trajectory_plan_valid);
    RUN_TEST(test_trajectory_plan_zero_velocity_fails);
    RUN_TEST(test_trajectory_plan_zero_acceleration_fails);
    RUN_TEST(test_trajectory_starts_idle);

    // Trapezoidal timing tests
    RUN_TEST(test_trajectory_timing_trapezoidal);
    RUN_TEST(test_trajectory_timing_triangular);
    RUN_TEST(test_trajectory_distance_sums_correctly);

    // Trapezoidal execution tests
    RUN_TEST(test_trajectory_start_changes_phase);
    RUN_TEST(test_trajectory_update_increases_velocity);
    RUN_TEST(test_trajectory_update_increases_position);
    RUN_TEST(test_trajectory_reaches_complete);
    RUN_TEST(test_trajectory_negative_distance);

    // Trapezoidal state query tests
    RUN_TEST(test_trajectory_get_velocity_at);
    RUN_TEST(test_trajectory_reset);

    // Trapezoidal string tests
    RUN_TEST(test_trajectory_phase_to_string);

    // Trapezoidal state struct tests
    RUN_TEST(test_trajectory_state_progress);

    // S-Curve planning tests
    RUN_TEST(test_scurve_plan_valid);
    RUN_TEST(test_scurve_plan_zero_jerk_fails);
    RUN_TEST(test_scurve_starts_idle);

    // S-Curve execution tests
    RUN_TEST(test_scurve_start_changes_phase);
    RUN_TEST(test_scurve_velocity_smooth_increase);
    RUN_TEST(test_scurve_reaches_complete);
    RUN_TEST(test_scurve_reduced_profile_short_move);

    // S-Curve state tests
    RUN_TEST(test_scurve_get_acceleration);
    RUN_TEST(test_scurve_phase_to_string);
    RUN_TEST(test_scurve_reset);

    UNITY_END();

    return 0;
}
