# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> **Maintained by:** Scribe Agent

---

## [Unreleased]

### Added
- *Nothing yet*

---

## [v0.2.0] - 2025-05-27

### Added

#### P1.1: IDriver Interface
- Abstract driver interface in `firmware/lib/usd_drivers/idriver.h`
- Methods: `enable()`, `disable()`, `step()`, `setDirection()`, `getPosition()`
- Fault detection and timing configuration support

#### P1.2: GenericStepDirDriver
- GPIO-based step/direction driver in `firmware/lib/usd_drivers/step_dir_driver.h/.cpp`
- Platform-independent with native stubs for testing
- 16 unit tests in `firmware/test/test_driver/`

#### P1.3: MCPWM Step Generator
- ESP32 MCPWM-based step pulse generator in `firmware/lib/usd_drivers/mcpwm_stepper.h/.cpp`
- Hardware-timed pulses from 1Hz to 500kHz
- Sub-microsecond jitter (hardware timing)
- 18 unit tests in `firmware/test/test_mcpwm/`

#### P1.5: MotionController
- Central motion state machine in `firmware/lib/usd_core/motion_controller.h/.cpp`
- States: IDLE → ACCEL → CRUISE → DECEL → HOLDING → ERROR
- Position mode (`moveTo`) and relative moves (`moveBy`)
- Emergency stop and fault handling
- 20 unit tests in `firmware/test/test_motion/`

#### P1.6: TrajectoryInterpolator
- Trapezoidal trajectory profiles in `firmware/lib/usd_core/trajectory.h/.cpp`
- Linear acceleration/deceleration with automatic triangular profile for short moves
- Look-ahead functions: `getVelocityAt()`, `getPositionAt()`
- 16 unit tests for trapezoidal profiles

#### P1.7: S-Curve Profile
- 7-phase jerk-limited acceleration profiles
- Phases: J+, A, J-, C, J-, D, J+ (smoother motion with reduced vibration)
- Automatic profile reduction for short moves
- 10 unit tests for S-curve behavior

### Changed
- Modified `firmware/lib/usd_drivers/library.json` frameworks to `"*"` for native test compatibility
- Removed `usd_drivers` from `lib_ignore` in native test environment

### Test Results
- **93 total tests** passing across 6 test suites
- Test suites: test_cobs (7), test_crc16 (6), test_driver (16), test_mcpwm (18), test_motion (20), test_trajectory (26)

---

## [v0.1.0] - 2025-05-26

### Added

#### P0.1: PlatformIO Setup
- ESP32-S3 target configuration at 240MHz
- Native test environment for host-side unit tests
- FreeRTOS flags and USB-CDC support

#### P0.2: FreeRTOS Task Skeleton
- 5-task architecture in `firmware/src/main.cpp`:
  - `taskComm` (Core 0, Priority 1) — Communication handling
  - `taskSensor` (Core 0, Priority 2) — Sensor polling
  - `taskSync` (Core 1, Priority 3) — Multi-MCU synchronization
  - `taskMotion` (Core 1, Priority 4) — Motion control
  - `taskSafety` (Core 1, Priority 5) — Safety monitoring
- Thread-safe logging with mutex protection

#### P0.3: Python Package Structure
- Created `software/usd/core/__init__.py`
- Created `software/usd/comm/__init__.py`
- Created `software/usd/devices/__init__.py`
- Created `software/usd/utils/__init__.py`
- Created `software/tests/conftest.py` with pytest fixtures
- Created `software/tests/test_package.py` for import verification
- Created `software/tests/README.md` with testing documentation

#### P0.4: Protocol Definitions
- C implementation in `common/protocol/`:
  - `cobs.h` / `cobs.c` — COBS byte stuffing
  - `crc16.h` / `crc16.c` — CRC-16-CCITT checksums
  - `messages.h` — Protocol message definitions
  - `README.md` — Protocol documentation
- Python implementation in `software/usd/comm/`:
  - `cobs.py` — COBS codec mirroring C implementation
  - `crc16.py` — CRC-16-CCITT matching firmware
  - `messages.py` — Protocol message classes

#### P0.5: Test Frameworks
- C tests in `firmware/test/`:
  - `test_cobs/test_cobs.cpp` — 7 COBS encode/decode tests
  - `test_crc16/test_crc16.cpp` — 6 CRC-16 verification tests
  - 13/13 native tests passing (requires MinGW on Windows)
- Python tests in `software/tests/`:
  - `test_protocol.py` — 22/22 tests passing, 92% coverage

#### P0.7: Development Environment
- `software/requirements.txt` — Python development dependencies
- `firmware/lib/usd_protocol/` — Local library copy of protocol files for ESP32 builds

#### P0.6: Debug Logging
- Thread-safe `usd_log()` function with mutex protection
- Thread-safe `usd_logf()` for formatted output
- Debug task statistics printed every 10 seconds

### Changed
- 

### Deprecated
- 

### Removed
- 

### Fixed
- 

### Security
- 

---

## [v0.0.0] - 2025-01-17

### Added
- Initial project structure
- SDD specification templates
- CopilotTribunal integration

---

<!-- Template for new versions:

## [vX.Y.Z] - YYYY-MM-DD

### Added
- New features

### Changed
- Changes in existing functionality

### Deprecated
- Soon-to-be removed features

### Removed
- Removed features

### Fixed
- Bug fixes

### Security
- Security fixes

-->

---

## Version History Summary

| Version | Date | Type | Highlights |
|---------|------|------|------------|
| v0.1.0 | 2025-05-26 | Phase | P0 Foundation complete |
| v0.0.0 | 2025-01-17 | Initial | Project setup |

---

## Related

- [ROADMAP.md](ROADMAP.md) — What's planned
- [KNOWN-ISSUES.md](KNOWN-ISSUES.md) — Known bugs
- [FEATURES/](FEATURES/) — Feature details
