/**
 * @file test_driver.cpp
 * @brief Unit tests for IDriver and GenericStepDirDriver
 *
 * Tests the driver interface and GPIO-based implementation
 * using PlatformIO's native test environment.
 */

#include <unity.h>
#include "step_dir_driver.h"

using namespace usd;

// ─────────────────────────────────────────────────────────────────────────────
// Test Fixtures
// ─────────────────────────────────────────────────────────────────────────────

static DriverPins test_pins = {
    .step_pin = 18,
    .dir_pin = 19,
    .enable_pin = 21,
    .enable_active_low = true,
    .dir_invert = false
};

static StepTiming test_timing = {
    .step_pulse_us = 3,
    .dir_setup_us = 10,
    .enable_delay_ms = 0  // Zero delay for fast tests
};

// ─────────────────────────────────────────────────────────────────────────────
// Driver State Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_driver_starts_disabled(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    TEST_ASSERT_FALSE(driver.isEnabled());
    TEST_ASSERT_EQUAL(DriverState::DISABLED, driver.getState());
}

void test_driver_enable_disable(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    // Enable
    bool result = driver.enable();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(driver.isEnabled());
    TEST_ASSERT_EQUAL(DriverState::ENABLED, driver.getState());
    
    // Disable
    driver.disable();
    TEST_ASSERT_FALSE(driver.isEnabled());
    TEST_ASSERT_EQUAL(DriverState::DISABLED, driver.getState());
}

void test_driver_initial_position_zero(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    TEST_ASSERT_EQUAL_INT32(0, driver.getPosition());
}

// ─────────────────────────────────────────────────────────────────────────────
// Direction Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_driver_default_direction_cw(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    TEST_ASSERT_EQUAL(Direction::CW, driver.getDirection());
}

void test_driver_set_direction(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    driver.setDirection(Direction::CCW);
    TEST_ASSERT_EQUAL(Direction::CCW, driver.getDirection());
    
    driver.setDirection(Direction::CW);
    TEST_ASSERT_EQUAL(Direction::CW, driver.getDirection());
}

// ─────────────────────────────────────────────────────────────────────────────
// Step Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_step_fails_when_disabled(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    bool result = driver.step();
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT32(0, driver.getPosition());  // Position unchanged
}

void test_step_increments_position_cw(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    driver.enable();
    driver.setDirection(Direction::CW);
    
    driver.step();
    TEST_ASSERT_EQUAL_INT32(1, driver.getPosition());
    
    driver.step();
    TEST_ASSERT_EQUAL_INT32(2, driver.getPosition());
}

void test_step_decrements_position_ccw(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    driver.enable();
    driver.setDirection(Direction::CCW);
    
    driver.step();
    TEST_ASSERT_EQUAL_INT32(-1, driver.getPosition());
    
    driver.step();
    TEST_ASSERT_EQUAL_INT32(-2, driver.getPosition());
}

void test_step_multiple_returns_count(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    driver.enable();
    
    uint32_t steps = driver.stepMultiple(10, 10000);  // 10 steps at 10kHz
    TEST_ASSERT_EQUAL_UINT32(10, steps);
    TEST_ASSERT_EQUAL_INT32(10, driver.getPosition());
}

void test_step_multiple_fails_when_disabled(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    uint32_t steps = driver.stepMultiple(10, 10000);
    TEST_ASSERT_EQUAL_UINT32(0, steps);
}

// ─────────────────────────────────────────────────────────────────────────────
// Position Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_set_position(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    driver.setPosition(1000);
    TEST_ASSERT_EQUAL_INT32(1000, driver.getPosition());
    
    driver.setPosition(-500);
    TEST_ASSERT_EQUAL_INT32(-500, driver.getPosition());
}

void test_reset_position(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    driver.setPosition(1000);
    driver.resetPosition();
    TEST_ASSERT_EQUAL_INT32(0, driver.getPosition());
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_get_pins(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    const DriverPins& pins = driver.getPins();
    TEST_ASSERT_EQUAL_INT8(18, pins.step_pin);
    TEST_ASSERT_EQUAL_INT8(19, pins.dir_pin);
    TEST_ASSERT_EQUAL_INT8(21, pins.enable_pin);
    TEST_ASSERT_TRUE(pins.enable_active_low);
    TEST_ASSERT_FALSE(pins.dir_invert);
}

void test_get_timing(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    const StepTiming& timing = driver.getTiming();
    TEST_ASSERT_EQUAL_UINT16(3, timing.step_pulse_us);
    TEST_ASSERT_EQUAL_UINT16(10, timing.dir_setup_us);
    TEST_ASSERT_EQUAL_UINT16(0, timing.enable_delay_ms);
}

// ─────────────────────────────────────────────────────────────────────────────
// Fault Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_no_fault_initially(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    TEST_ASSERT_FALSE(driver.isFault());
    TEST_ASSERT_EQUAL_UINT8(0, driver.getFaultCode());
}

void test_clear_fault_returns_false_when_no_fault(void)
{
    GenericStepDirDriver driver(test_pins, test_timing);
    
    // No fault to clear
    bool result = driver.clearFault();
    TEST_ASSERT_FALSE(result);
}

// ─────────────────────────────────────────────────────────────────────────────
// Test Runner
// ─────────────────────────────────────────────────────────────────────────────

void setUp(void)
{
    // Setup before each test (if needed)
}

void tearDown(void)
{
    // Cleanup after each test (if needed)
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();

    // Driver state tests
    RUN_TEST(test_driver_starts_disabled);
    RUN_TEST(test_driver_enable_disable);
    RUN_TEST(test_driver_initial_position_zero);

    // Direction tests
    RUN_TEST(test_driver_default_direction_cw);
    RUN_TEST(test_driver_set_direction);

    // Step tests
    RUN_TEST(test_step_fails_when_disabled);
    RUN_TEST(test_step_increments_position_cw);
    RUN_TEST(test_step_decrements_position_ccw);
    RUN_TEST(test_step_multiple_returns_count);
    RUN_TEST(test_step_multiple_fails_when_disabled);

    // Position tests
    RUN_TEST(test_set_position);
    RUN_TEST(test_reset_position);

    // Configuration tests
    RUN_TEST(test_get_pins);
    RUN_TEST(test_get_timing);

    // Fault tests
    RUN_TEST(test_no_fault_initially);
    RUN_TEST(test_clear_fault_returns_false_when_no_fault);

    UNITY_END();

    return 0;
}
