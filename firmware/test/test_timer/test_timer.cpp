/**
 * @file test_timer.cpp
 * @brief Unit tests for TimerStepper
 *
 * Tests timer-based step generator logic (native stubs for hardware ops)
 */

#include <unity.h>
#include "timer_stepper.h"

using namespace usd;

// ─────────────────────────────────────────────────────────────────────────────
// Test Fixtures
// ─────────────────────────────────────────────────────────────────────────────

static TimerConfig test_config = {
    .timer_name = "test_timer",
    .skip_unhandled = true
};

// ─────────────────────────────────────────────────────────────────────────────
// Initialization Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_timer_init(void)
{
    TimerStepper stepper(18, test_config);
    bool result = stepper.init();
    TEST_ASSERT_TRUE(result);
}

void test_timer_starts_idle(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    TEST_ASSERT_FALSE(stepper.isRunning());
    TEST_ASSERT_EQUAL(StepGenState::IDLE, stepper.getState());
}

void test_timer_get_step_pin(void)
{
    TimerStepper stepper(18, test_config);
    TEST_ASSERT_EQUAL_INT8(18, stepper.getStepPin());
}

void test_timer_get_pulse_width(void)
{
    TimerStepper stepper(18, test_config, 5);
    TEST_ASSERT_EQUAL_UINT16(5, stepper.getPulseWidth());
}

// ─────────────────────────────────────────────────────────────────────────────
// Frequency Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_timer_set_frequency_valid(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(10000);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(10000, stepper.getFrequency());
}

void test_timer_set_frequency_min(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(1);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(1, stepper.getFrequency());
}

void test_timer_set_frequency_max(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    // TimerStepper max is 50kHz (vs MCPWM's 500kHz)
    bool result = stepper.setFrequency(50000);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(50000, stepper.getFrequency());
}

void test_timer_set_frequency_zero_fails(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(0);
    TEST_ASSERT_FALSE(result);
}

void test_timer_set_frequency_over_max_fails(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    // TimerStepper max is 50kHz
    bool result = stepper.setFrequency(50001);
    TEST_ASSERT_FALSE(result);
}

void test_timer_frequency_limits(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    TEST_ASSERT_EQUAL_UINT32(1, stepper.getMinFrequency());
    TEST_ASSERT_EQUAL_UINT32(50000, stepper.getMaxFrequency());
}

// ─────────────────────────────────────────────────────────────────────────────
// Start/Stop Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_timer_start_without_frequency_fails(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    // Frequency not set (0)
    bool result = stepper.start();
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(stepper.isRunning());
}

void test_timer_start_without_init_fails(void)
{
    TimerStepper stepper(18, test_config);
    // Not initialized
    stepper.setFrequency(10000);
    
    bool result = stepper.start();
    TEST_ASSERT_FALSE(result);
}

void test_timer_start_stop_cycle(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    stepper.setFrequency(10000);
    
    // Start
    bool result = stepper.start();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(stepper.isRunning());
    TEST_ASSERT_EQUAL(StepGenState::RUNNING, stepper.getState());
    
    // Stop
    stepper.stop();
    TEST_ASSERT_FALSE(stepper.isRunning());
    TEST_ASSERT_EQUAL(StepGenState::IDLE, stepper.getState());
}

void test_timer_stop_when_idle_is_safe(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    // Should not crash when stopping while already idle
    stepper.stop();
    TEST_ASSERT_FALSE(stepper.isRunning());
}

// ─────────────────────────────────────────────────────────────────────────────
// Step Counting Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_timer_step_count_starts_zero(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    TEST_ASSERT_EQUAL_UINT32(0, stepper.getStepCount());
}

void test_timer_reset_step_count(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    stepper.resetStepCount();
    TEST_ASSERT_EQUAL_UINT32(0, stepper.getStepCount());
}

void test_timer_set_target_steps(void)
{
    TimerStepper stepper(18, test_config);
    stepper.init();
    
    stepper.setTargetSteps(1000);
    // Just verify it doesn't crash - target is internal
    TEST_PASS();
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_timer_get_config(void)
{
    TimerStepper stepper(18, test_config);
    
    const TimerConfig& config = stepper.getConfig();
    TEST_ASSERT_EQUAL_STRING("test_timer", config.timer_name);
    TEST_ASSERT_TRUE(config.skip_unhandled);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test Runner
// ─────────────────────────────────────────────────────────────────────────────

void setUp(void) {}
void tearDown(void) {}

int main(int, char**)
{
    UNITY_BEGIN();

    // Initialization tests
    RUN_TEST(test_timer_init);
    RUN_TEST(test_timer_starts_idle);
    RUN_TEST(test_timer_get_step_pin);
    RUN_TEST(test_timer_get_pulse_width);

    // Frequency tests
    RUN_TEST(test_timer_set_frequency_valid);
    RUN_TEST(test_timer_set_frequency_min);
    RUN_TEST(test_timer_set_frequency_max);
    RUN_TEST(test_timer_set_frequency_zero_fails);
    RUN_TEST(test_timer_set_frequency_over_max_fails);
    RUN_TEST(test_timer_frequency_limits);

    // Start/Stop tests
    RUN_TEST(test_timer_start_without_frequency_fails);
    RUN_TEST(test_timer_start_without_init_fails);
    RUN_TEST(test_timer_start_stop_cycle);
    RUN_TEST(test_timer_stop_when_idle_is_safe);

    // Step counting tests
    RUN_TEST(test_timer_step_count_starts_zero);
    RUN_TEST(test_timer_reset_step_count);
    RUN_TEST(test_timer_set_target_steps);

    // Configuration tests
    RUN_TEST(test_timer_get_config);

    UNITY_END();

    return 0;
}
