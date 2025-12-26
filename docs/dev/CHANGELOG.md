# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> **Maintained by:** Scribe Agent

---

## [Unreleased]

### Added
- Complete SDD specification suite (2025-01-17)
  - DISCOVERY.md: Vision, goals, constraints, MoSCoW scope
  - REQUIREMENTS.md: 66 requirements (34 Must, 25 Should, 7 Could)
  - ARCHITECTURE.md: System design, protocols, firmware/software structure
  - TASKS.md: 10-phase implementation plan (~234 hours)
- Project folder structure
  - `firmware/` with PlatformIO configuration and library stubs
  - `software/` with Python package structure (pyproject.toml)
  - `common/`, `demos/`, `scripts/` directories
- Development tracking (ROADMAP.md with milestones)

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
