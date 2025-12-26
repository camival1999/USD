# Feature: P0 Foundation

| Field | Value |
|-------|-------|
| **Status** | ✅ Complete |
| **Priority** | High |
| **Started** | 2025-05-26 |
| **Completed** | 2025-05-26 |
| **Version** | v0.1.0 |
| **Related Issues** | — |

## Description

Phase P0 establishes the foundational build system, FreeRTOS task structure, and test frameworks for both the firmware and host software components of the Ultimate Stepper Driver project.

## Requirements

This phase addresses the infrastructure requirements needed before feature development:
- Build system configuration for ESP32-S3
- Real-time task architecture
- Protocol layer foundations
- Test framework setup

## Tasks Completed

### P0.1: PlatformIO Setup
- ESP32-S3 target configuration at 240MHz
- Native test environment for host-side unit tests
- FreeRTOS flags and USB-CDC support enabled

### P0.2: FreeRTOS Task Skeleton
5-task architecture implemented:
| Task | Core | Priority | Purpose |
|------|------|----------|---------|
| taskComm | 0 | 1 | Communication handling |
| taskSensor | 0 | 2 | Sensor polling |
| taskSync | 1 | 3 | Multi-MCU synchronization |
| taskMotion | 1 | 4 | Motion control |
| taskSafety | 1 | 5 | Safety monitoring |

### P0.3: Python Package Structure
- `software/usd/core/` — Core motor control logic
- `software/usd/comm/` — Communication/protocol layer
- `software/usd/devices/` — Device abstractions
- `software/usd/utils/` — Utility functions
- `software/tests/` — Test suite with fixtures

### P0.4: Protocol Definitions
Mirrored implementations in C and Python:
- **COBS** — Byte stuffing with 0x00 frame delimiters
- **CRC-16-CCITT** — Polynomial 0x1021, initial 0xFFFF
- **Messages** — Protocol message type definitions

### P0.5: Test Frameworks
- **C/C++**: PlatformIO native test environment
  - `test_cobs.cpp` — COBS encode/decode verification
  - `test_crc16.cpp` — CRC-16 checksum tests
- **Python**: pytest with conftest fixtures
  - `test_protocol.py` — 22/22 tests passing, 92% coverage

### P0.6: Debug Logging
- Thread-safe `usd_log()` with mutex protection
- Formatted `usd_logf()` for printf-style output
- Debug task statistics every 10 seconds

## Files Changed

### Firmware (`firmware/`)
- `platformio.ini` — ESP32-S3 configuration, native test env
- `src/main.cpp` — FreeRTOS 5-task architecture with logging

### Common Protocol (`common/protocol/`)
- `cobs.h` / `cobs.c` — COBS implementation
- `crc16.h` / `crc16.c` — CRC-16-CCITT implementation
- `messages.h` — Protocol message definitions
- `README.md` — Protocol documentation

### Firmware Tests (`firmware/test/`)
- `test_cobs/test_cobs.cpp` — COBS unit tests (7 tests)
- `test_crc16/test_crc16.cpp` — CRC-16 unit tests (6 tests)

### Firmware Library (`firmware/lib/usd_protocol/`)
- Copy of `common/protocol/` for PlatformIO builds
- `library.json` — PlatformIO library manifest
- `usd_protocol.h` — Unified include header

### Python Package (`software/usd/`)
- `core/__init__.py` — Core module initialization
- `comm/__init__.py` — Comm module initialization
- `comm/cobs.py` — COBS codec
- `comm/crc16.py` — CRC-16 implementation
- `comm/messages.py` — Message classes
- `devices/__init__.py` — Devices module initialization
- `utils/__init__.py` — Utils module initialization

### Python Tests (`software/tests/`)
- `conftest.py` — pytest fixtures
- `test_package.py` — Package import tests
- `test_protocol.py` — Protocol tests (22 tests, 92% coverage)
- `README.md` — Test documentation

## Testing

| Test Suite | Tests | Passing | Coverage |
|------------|-------|---------|----------|
| Python protocol | 22 | 22 | 92% |
| C COBS | 7 | 7 | — |
| C CRC-16 | 6 | 6 | — |
| **Total** | **35** | **35** | — |

> **Note:** C native tests require MinGW on Windows (`mingw32-make` toolchain).

## Notes

- Core 0 handles communication-bound tasks (comm, sensor)
- Core 1 handles time-critical tasks (sync, motion, safety)
- Higher priority numbers = higher priority (safety is highest)
- Protocol implementations are byte-for-byte compatible between C and Python

## Next Phase

**P1: Core Motion** — Open-loop motor control with trapezoidal and S-curve profiles
