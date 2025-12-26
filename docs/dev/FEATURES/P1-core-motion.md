# Feature: P1 Core Motion

| Field | Value |
|-------|-------|
| **Status** | ✅ Complete |
| **Priority** | High |
| **Started** | 2025-05-27 |
| **Completed** | 2025-05-27 |
| **Version** | v0.2.0 |
| **Related Issues** | — |

## Description

Phase P1 implements the core motion control stack for open-loop stepper motor control. This includes the hardware abstraction layer for step/dir drivers, MCPWM-based step pulse generation, motion state machine, and trajectory generation with both trapezoidal and S-curve profiles.

## Requirements Addressed

From REQUIREMENTS.md:
- **FR-001**: Stepper pulse generation (1Hz - 500kHz via MCPWM)
- **FR-002**: Acceleration profile support (trapezoidal + S-curve)
- **NFR-004**: Step timing jitter (< 100ns with MCPWM hardware)
- **NFR-008**: Motion profiles with position/velocity/time modes

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        MotionController                          │
│  (State machine: IDLE → ACCEL → CRUISE → DECEL → HOLDING)       │
└──────────────────────────┬──────────────────────────────────────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
          ▼                ▼                ▼
   ┌────────────┐   ┌────────────┐   ┌────────────────┐
   │  IDriver   │   │ McpwmStepper│  │   Trajectory   │
   │ (abstract) │   │ (step gen)  │  │ (profile gen)  │
   └─────┬──────┘   └─────┬──────┘   └───────┬────────┘
         │                │                   │
         ▼                ▼                   ▼
┌────────────────┐  ┌──────────┐    ┌──────────────────┐
│GenericStepDir  │  │  MCPWM   │    │ Trapezoidal  │ S │
│    Driver      │  │ Hardware │    │   Profile    │ C │
└────────────────┘  └──────────┘    └──────────────────┘
```

## Tasks Completed

### P1.1: IDriver Interface
- Abstract base class for motor drivers
- Methods: `enable()`, `disable()`, `step()`, `setDirection()`, `getPosition()`
- Supports fault detection and timing configuration

### P1.2: GenericStepDirDriver
- GPIO-based step/direction driver implementation
- Platform-independent with native stubs for testing
- **16 unit tests** covering all functionality

### P1.3: MCPWM Step Generator
- ESP32 MCPWM peripheral for hardware-timed pulses
- Frequency range: 1Hz to 500kHz
- Sub-microsecond jitter (hardware timing)
- **18 unit tests** for configuration and state management

### P1.4: Timer ISR Fallback (DEFERRED)
- Software timer fallback - not needed with MCPWM
- Can be added later for non-ESP32 platforms

### P1.5: MotionController
- Central motion state machine
- States: IDLE → ACCEL → CRUISE → DECEL → HOLDING → ERROR
- Supports position mode (`moveTo`) and relative moves (`moveBy`)
- Emergency stop and fault handling
- **20 unit tests** for state machine behavior

### P1.6: TrajectoryInterpolator (Trapezoidal)
- Linear acceleration/deceleration profiles
- Automatic triangular profile for short moves
- Phase-based timing: ACCEL → CRUISE → DECEL
- Look-ahead functions: `getVelocityAt()`, `getPositionAt()`
- **16 unit tests** for planning and execution

### P1.7: S-Curve Profile
- 7-phase jerk-limited acceleration
- Phases: J+, A, J-, C, J-, D, J+ (jerk, accel, jerk, cruise, jerk, decel, jerk)
- Automatic profile reduction for short moves
- Smoother motion with reduced vibration
- **10 unit tests** for S-curve behavior

### P1.8: Motion Unit Tests
- **93 total tests** passing across all test suites
- Exceeds 80% coverage target

## Files Created

### usd_drivers Library
| File | Lines | Purpose |
|------|-------|---------|
| `idriver.h` | ~120 | Abstract driver interface |
| `step_dir_driver.h` | ~90 | GPIO driver header |
| `step_dir_driver.cpp` | ~150 | GPIO driver implementation |
| `mcpwm_stepper.h` | ~120 | MCPWM step generator header |
| `mcpwm_stepper.cpp` | ~180 | MCPWM implementation |
| `library.json` | ~15 | PlatformIO library config |
| `README.md` | ~80 | Library documentation |

### usd_core Library
| File | Lines | Purpose |
|------|-------|---------|
| `motion_controller.h` | ~170 | Motion state machine header |
| `motion_controller.cpp` | ~270 | Motion controller implementation |
| `trajectory.h` | ~450 | Trajectory classes header |
| `trajectory.cpp` | ~700 | Trapezoidal + S-curve implementation |
| `library.json` | ~15 | PlatformIO library config |
| `README.md` | ~100 | Library documentation |

### Test Files
| File | Tests | Purpose |
|------|-------|---------|
| `test_driver.cpp` | 16 | Driver interface tests |
| `test_mcpwm.cpp` | 18 | MCPWM stepper tests |
| `test_motion.cpp` | 20 | Motion controller tests |
| `test_trajectory.cpp` | 26 | Trajectory tests (16 trap + 10 S-curve) |

## Test Results

```
Test Suite       Tests   Status
─────────────────────────────────
test_cobs          7     PASSED
test_crc16         6     PASSED
test_driver       16     PASSED
test_mcpwm        18     PASSED
test_motion       20     PASSED
test_trajectory   26     PASSED
─────────────────────────────────
TOTAL             93     PASSED
```

## Build Status

| Target | Flash | RAM | Status |
|--------|-------|-----|--------|
| ESP32-S3 | 4.1% | 5.8% | ✅ |
| Native (tests) | N/A | N/A | ✅ |

## Implementation Notes

### MCPWM vs Timer ISR
- MCPWM provides hardware-timed pulses with zero CPU overhead during stepping
- Maximum frequency 500kHz (limited by stepper driver requirements)
- Timer ISR fallback deferred as MCPWM covers all use cases

### S-Curve Complexity
- 7-phase profile requires careful phase boundary tracking
- Binary search used for short-move velocity optimization
- Phase transitions handled by cumulative time comparisons

### Native Testing Strategy
- Changed `library.json` frameworks to `"*"` for native compatibility
- Platform-specific code guarded by `#ifdef ARDUINO` / `#ifdef ESP_PLATFORM`
- Stub implementations for GPIO and MCPWM calls in native env

## Next Steps

Phase P2: Closed-Loop Control
- Quadrature encoder interface
- Position feedback loop
- Stall detection algorithms
