# USD Core Library

Core motion control algorithms for the Ultimate Stepper Driver.

## Components

| Component | File | Description |
|-----------|------|-------------|
| **MotionController** | `motion_controller.h/.cpp` | State machine, open-loop commands |
| **TrajectoryInterpolator** | `trajectory.h/.cpp` | Motion profiles (trapezoid, S-curve) |
| **PIDController** | `pid_controller.h/.cpp` | Closed-loop control (P3) |

## MotionController

The central orchestrator for motor motion:

```cpp
#include "motion_controller.h"
#include "step_dir_driver.h"
#include "mcpwm_stepper.h"

usd::GenericStepDirDriver driver(pins, timing);
usd::McpwmStepper stepper(18, mcpwm_config);
usd::MotionController controller;

controller.attachDriver(&driver);
controller.attachStepper(&stepper);
controller.enable();

// Simple moves
controller.moveTo(10000);        // Absolute position
controller.moveBy(-5000);        // Relative move

// Full parameter control
usd::MotionParams params = {
    .target_position = 50000,
    .max_velocity = 100000,
    .acceleration = 200000,
    .profile = usd::ProfileType::TRAPEZOIDAL,
    .move_type = usd::MoveType::ABSOLUTE
};
controller.startMove(params);

// Control loop (call from MotionTask)
while (!controller.isAtTarget()) {
    controller.tick(dt_us);
}
```

## Motion States

```
IDLE → ACCELERATING → CRUISING → DECELERATING → HOLDING
  ↑                                                  │
  └──────────────────────────────────────────────────┘
```

| State | Description |
|-------|-------------|
| `IDLE` | Stopped, ready for commands |
| `ACCELERATING` | Ramping up to target velocity |
| `CRUISING` | At target velocity |
| `DECELERATING` | Ramping down to stop |
| `HOLDING` | At position, motor enabled |
| `FAULT` | Error condition |
| `HOMING` | Homing sequence (P4) |

## Trajectory Profiles

### Trapezoidal Profile

Linear acceleration with optional cruise phase:

```cpp
#include "trajectory.h"

usd::TrapezoidalTrajectory traj;

usd::TrajectoryParams params = {
    .distance = 10000,
    .max_velocity = 50000,
    .acceleration = 100000,
    .deceleration = 100000
};

traj.plan(params);
traj.start();

while (!traj.isComplete()) {
    usd::TrajectoryState state = traj.update(dt_us);
    stepper.setFrequency(state.velocity);
}
```

### S-Curve Profile

7-phase jerk-limited profile for smoother motion:

```cpp
usd::SCurveTrajectory traj;

usd::SCurveParams params = {
    .distance = 10000,
    .max_velocity = 50000,
    .max_acceleration = 100000,
    .max_jerk = 500000
};

traj.plan(params);
traj.start();

while (!traj.isComplete()) {
    usd::SCurveState state = traj.update(dt_us);
    stepper.setFrequency(state.velocity);
    // state.acceleration is also available for monitoring
}
```

## Status

| Component | Status |
|-----------|--------|
| MotionController | ✅ Complete (P1.5) |
| TrajectoryInterpolator | ✅ Complete (P1.6) |
| S-Curve Profile | ✅ Complete (P1.7) |
| PIDController | 📋 P3.x |

## Dependencies

- `usd_drivers` (IDriver, McpwmStepper)
- FreeRTOS (for timing)

## Related

- [Architecture](../../../docs/specs/ARCHITECTURE.md)
- [TASKS](../../../docs/specs/TASKS.md)
