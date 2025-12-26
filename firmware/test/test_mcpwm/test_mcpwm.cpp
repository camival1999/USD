/**
 * @file test_mcpwm.cpp
 * @brief Unit tests for McpwmStepper
 *
 * Tests MCPWM step generator logic (native stubs for hardware ops)
 */

#include <unity.h>
#include "mcpwm_stepper.h"

using namespace usd;

// ─────────────────────────────────────────────────────────────────────────────
// Test Fixtures
// ─────────────────────────────────────────────────────────────────────────────

static McpwmConfig test_config = {
    .unit = 0,
    .timer = 0,
    .operator_id = 0,
    .output = 0
};

// ─────────────────────────────────────────────────────────────────────────────
// Initialization Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_mcpwm_init(void)
{
    McpwmStepper stepper(18, test_config);
    bool result = stepper.init();
    TEST_ASSERT_TRUE(result);
}

void test_mcpwm_starts_idle(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    TEST_ASSERT_FALSE(stepper.isRunning());
    TEST_ASSERT_EQUAL(StepGenState::IDLE, stepper.getState());
}

void test_mcpwm_get_step_pin(void)
{
    McpwmStepper stepper(18, test_config);
    TEST_ASSERT_EQUAL_INT8(18, stepper.getStepPin());
}

void test_mcpwm_get_pulse_width(void)
{
    McpwmStepper stepper(18, test_config, 5);
    TEST_ASSERT_EQUAL_UINT16(5, stepper.getPulseWidth());
}

// ─────────────────────────────────────────────────────────────────────────────
// Frequency Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_mcpwm_set_frequency_valid(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(10000);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(10000, stepper.getFrequency());
}

void test_mcpwm_set_frequency_min(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(1);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(1, stepper.getFrequency());
}

void test_mcpwm_set_frequency_max(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(500000);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(500000, stepper.getFrequency());
}

void test_mcpwm_set_frequency_zero_fails(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(0);
    TEST_ASSERT_FALSE(result);
}

void test_mcpwm_set_frequency_over_max_fails(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    bool result = stepper.setFrequency(500001);
    TEST_ASSERT_FALSE(result);
}

void test_mcpwm_frequency_limits(void)
{
    TEST_ASSERT_EQUAL_UINT32(1, McpwmStepper::getMinFrequency());
    TEST_ASSERT_EQUAL_UINT32(500000, McpwmStepper::getMaxFrequency());
}

// ─────────────────────────────────────────────────────────────────────────────
// Start/Stop Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_mcpwm_start_without_frequency_fails(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    // Frequency not set (0)
    bool result = stepper.start();
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(stepper.isRunning());
}

void test_mcpwm_start_without_init_fails(void)
{
    McpwmStepper stepper(18, test_config);
    // Not initialized
    stepper.setFrequency(10000);
    
    bool result = stepper.start();
    TEST_ASSERT_FALSE(result);
}

void test_mcpwm_start_stop_cycle(void)
{
    McpwmStepper stepper(18, test_config);
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

void test_mcpwm_stop_when_idle_is_safe(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    // Should not crash when stopping while already idle
    stepper.stop();
    TEST_ASSERT_FALSE(stepper.isRunning());
}

// ─────────────────────────────────────────────────────────────────────────────
// Step Counting Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_mcpwm_step_count_starts_zero(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    TEST_ASSERT_EQUAL_UINT32(0, stepper.getStepCount());
}

void test_mcpwm_reset_step_count(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    stepper.resetStepCount();
    TEST_ASSERT_EQUAL_UINT32(0, stepper.getStepCount());
}

void test_mcpwm_set_target_steps(void)
{
    McpwmStepper stepper(18, test_config);
    stepper.init();
    
    stepper.setTargetSteps(1000);
    // Just verify it doesn't crash - target is internal
    TEST_PASS();
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_mcpwm_get_config(void)
{
    McpwmStepper stepper(18, test_config);
    
    const McpwmConfig& config = stepper.getConfig();
    TEST_ASSERT_EQUAL_UINT8(0, config.unit);
    TEST_ASSERT_EQUAL_UINT8(0, config.timer);
    TEST_ASSERT_EQUAL_UINT8(0, config.operator_id);
    TEST_ASSERT_EQUAL_UINT8(0, config.output);
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
    RUN_TEST(test_mcpwm_init);
    RUN_TEST(test_mcpwm_starts_idle);
    RUN_TEST(test_mcpwm_get_step_pin);
    RUN_TEST(test_mcpwm_get_pulse_width);

    // Frequency tests
    RUN_TEST(test_mcpwm_set_frequency_valid);
    RUN_TEST(test_mcpwm_set_frequency_min);
    RUN_TEST(test_mcpwm_set_frequency_max);
    RUN_TEST(test_mcpwm_set_frequency_zero_fails);
    RUN_TEST(test_mcpwm_set_frequency_over_max_fails);
    RUN_TEST(test_mcpwm_frequency_limits);

    // Start/Stop tests
    RUN_TEST(test_mcpwm_start_without_frequency_fails);
    RUN_TEST(test_mcpwm_start_without_init_fails);
    RUN_TEST(test_mcpwm_start_stop_cycle);
    RUN_TEST(test_mcpwm_stop_when_idle_is_safe);

    // Step counting tests
    RUN_TEST(test_mcpwm_step_count_starts_zero);
    RUN_TEST(test_mcpwm_reset_step_count);
    RUN_TEST(test_mcpwm_set_target_steps);

    // Configuration tests
    RUN_TEST(test_mcpwm_get_config);

    UNITY_END();

    return 0;
}
