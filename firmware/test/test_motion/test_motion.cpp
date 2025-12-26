/**
 * @file test_motion.cpp
 * @brief Unit tests for MotionController
 *
 * Tests motion controller state machine and commands (native stubs for hardware)
 */

#include <unity.h>
#include "motion_controller.h"
#include "step_dir_driver.h"
#include "mcpwm_stepper.h"

using namespace usd;

// ─────────────────────────────────────────────────────────────────────────────
// Test Fixtures
// ─────────────────────────────────────────────────────────────────────────────

static DriverPins test_driver_pins = {
    .step_pin = 18,
    .dir_pin = 19,
    .enable_pin = 21,
    .enable_active_low = true,
    .dir_invert = false
};

static StepTiming test_driver_timing = {
    .step_pulse_us = 3,
    .dir_setup_us = 10,
    .enable_delay_ms = 0
};

static McpwmConfig test_mcpwm_config = {
    .unit = 0,
    .timer = 0,
    .operator_id = 0,
    .output = 0
};

// ─────────────────────────────────────────────────────────────────────────────
// Initialization Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_controller_starts_idle(void)
{
    MotionController controller;
    
    TEST_ASSERT_EQUAL(MotionState::IDLE, controller.getState());
    TEST_ASSERT_FALSE(controller.isMoving());
    TEST_ASSERT_FALSE(controller.isEnabled());
}

void test_motion_controller_hardware_not_attached_initially(void)
{
    MotionController controller;
    
    TEST_ASSERT_FALSE(controller.isHardwareAttached());
}

void test_motion_controller_attach_hardware(void)
{
    // Declare driver and stepper BEFORE controller so they outlive it
    // (destructors run in reverse order of declaration)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    McpwmStepper stepper(18, test_mcpwm_config);
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.attachStepper(&stepper);
    
    TEST_ASSERT_TRUE(controller.isHardwareAttached());
}

void test_motion_controller_initial_position_zero(void)
{
    MotionController controller;
    
    TEST_ASSERT_EQUAL_INT32(0, controller.getPosition());
}

// ─────────────────────────────────────────────────────────────────────────────
// Enable/Disable Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_controller_enable_without_driver_fails(void)
{
    MotionController controller;
    
    bool result = controller.enable();
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(controller.isEnabled());
}

void test_motion_controller_enable_with_driver(void)
{
    // Declare driver BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    MotionController controller;
    
    controller.attachDriver(&driver);
    
    bool result = controller.enable();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(controller.isEnabled());
}

void test_motion_controller_disable(void)
{
    // Declare driver BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.enable();
    
    controller.disable();
    TEST_ASSERT_FALSE(controller.isEnabled());
}

// ─────────────────────────────────────────────────────────────────────────────
// Move Command Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_controller_move_without_hardware_fails(void)
{
    MotionController controller;
    
    bool result = controller.moveTo(1000);
    TEST_ASSERT_FALSE(result);
}

void test_motion_controller_move_to_starts_motion(void)
{
    // Declare driver and stepper BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    McpwmStepper stepper(18, test_mcpwm_config);
    stepper.init();
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.attachStepper(&stepper);
    
    bool result = controller.moveTo(1000);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(MotionState::ACCELERATING, controller.getState());
    TEST_ASSERT_TRUE(controller.isMoving());
}

void test_motion_controller_move_by_relative(void)
{
    // Declare driver and stepper BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    McpwmStepper stepper(18, test_mcpwm_config);
    stepper.init();
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.attachStepper(&stepper);
    controller.setPosition(500);
    
    bool result = controller.moveBy(100);
    TEST_ASSERT_TRUE(result);
    
    // Target should be current + distance
    MotionStatus status = controller.getStatus();
    TEST_ASSERT_EQUAL_INT32(600, status.target_position);
}

void test_motion_controller_move_to_same_position_goes_to_holding(void)
{
    // Declare driver and stepper BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    McpwmStepper stepper(18, test_mcpwm_config);
    stepper.init();
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.attachStepper(&stepper);
    controller.setPosition(1000);
    
    // Move to current position
    bool result = controller.moveTo(1000);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(MotionState::HOLDING, controller.getState());
}

// ─────────────────────────────────────────────────────────────────────────────
// Status Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_controller_get_status(void)
{
    MotionController controller;
    controller.setPosition(500);
    
    MotionStatus status = controller.getStatus();
    TEST_ASSERT_EQUAL(MotionState::IDLE, status.state);
    TEST_ASSERT_EQUAL_INT32(500, status.current_position);
    TEST_ASSERT_FALSE(status.in_motion);
}

void test_motion_controller_distance_to_go(void)
{
    // Declare driver and stepper BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    McpwmStepper stepper(18, test_mcpwm_config);
    stepper.init();
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.attachStepper(&stepper);
    controller.setPosition(0);
    controller.moveTo(1000);
    
    TEST_ASSERT_EQUAL_INT32(1000, controller.getDistanceToGo());
}

// ─────────────────────────────────────────────────────────────────────────────
// Position Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_controller_set_position(void)
{
    MotionController controller;
    
    controller.setPosition(5000);
    TEST_ASSERT_EQUAL_INT32(5000, controller.getPosition());
}

void test_motion_controller_reset_position(void)
{
    MotionController controller;
    
    controller.setPosition(5000);
    controller.resetPosition();
    TEST_ASSERT_EQUAL_INT32(0, controller.getPosition());
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_controller_set_velocity(void)
{
    MotionController controller;
    
    controller.setMaxVelocity(50000);
    TEST_ASSERT_EQUAL_UINT32(50000, controller.getConfig().default_velocity);
}

void test_motion_controller_set_acceleration(void)
{
    MotionController controller;
    
    controller.setAcceleration(100000);
    TEST_ASSERT_EQUAL_UINT32(100000, controller.getConfig().default_acceleration);
}

// ─────────────────────────────────────────────────────────────────────────────
// Stop Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_controller_emergency_stop(void)
{
    // Declare driver and stepper BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    McpwmStepper stepper(18, test_mcpwm_config);
    stepper.init();
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.attachStepper(&stepper);
    controller.moveTo(1000);
    
    controller.emergencyStop();
    
    TEST_ASSERT_EQUAL(MotionState::IDLE, controller.getState());
    TEST_ASSERT_FALSE(controller.isMoving());
    TEST_ASSERT_EQUAL_UINT32(0, controller.getVelocity());
}

void test_motion_controller_stop_starts_deceleration(void)
{
    // Declare driver and stepper BEFORE controller (destructors run in reverse order)
    GenericStepDirDriver driver(test_driver_pins, test_driver_timing);
    McpwmStepper stepper(18, test_mcpwm_config);
    stepper.init();
    MotionController controller;
    
    controller.attachDriver(&driver);
    controller.attachStepper(&stepper);
    controller.moveTo(10000);
    
    controller.stop();
    
    TEST_ASSERT_EQUAL(MotionState::DECELERATING, controller.getState());
}

// ─────────────────────────────────────────────────────────────────────────────
// State String Tests
// ─────────────────────────────────────────────────────────────────────────────

void test_motion_state_to_string(void)
{
    TEST_ASSERT_EQUAL_STRING("IDLE", motionStateToString(MotionState::IDLE));
    TEST_ASSERT_EQUAL_STRING("ACCELERATING", motionStateToString(MotionState::ACCELERATING));
    TEST_ASSERT_EQUAL_STRING("CRUISING", motionStateToString(MotionState::CRUISING));
    TEST_ASSERT_EQUAL_STRING("DECELERATING", motionStateToString(MotionState::DECELERATING));
    TEST_ASSERT_EQUAL_STRING("HOLDING", motionStateToString(MotionState::HOLDING));
    TEST_ASSERT_EQUAL_STRING("FAULT", motionStateToString(MotionState::FAULT));
    TEST_ASSERT_EQUAL_STRING("HOMING", motionStateToString(MotionState::HOMING));
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
    RUN_TEST(test_motion_controller_starts_idle);
    RUN_TEST(test_motion_controller_hardware_not_attached_initially);
    RUN_TEST(test_motion_controller_attach_hardware);
    RUN_TEST(test_motion_controller_initial_position_zero);

    // Enable/Disable tests
    RUN_TEST(test_motion_controller_enable_without_driver_fails);
    RUN_TEST(test_motion_controller_enable_with_driver);
    RUN_TEST(test_motion_controller_disable);

    // Move command tests
    RUN_TEST(test_motion_controller_move_without_hardware_fails);
    RUN_TEST(test_motion_controller_move_to_starts_motion);
    RUN_TEST(test_motion_controller_move_by_relative);
    RUN_TEST(test_motion_controller_move_to_same_position_goes_to_holding);

    // Status tests
    RUN_TEST(test_motion_controller_get_status);
    RUN_TEST(test_motion_controller_distance_to_go);

    // Position tests
    RUN_TEST(test_motion_controller_set_position);
    RUN_TEST(test_motion_controller_reset_position);

    // Configuration tests
    RUN_TEST(test_motion_controller_set_velocity);
    RUN_TEST(test_motion_controller_set_acceleration);

    // Stop tests
    RUN_TEST(test_motion_controller_emergency_stop);
    RUN_TEST(test_motion_controller_stop_starts_deceleration);

    // State string tests
    RUN_TEST(test_motion_state_to_string);

    UNITY_END();

    return 0;
}
