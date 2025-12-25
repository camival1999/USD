# USD Requirements Specification

> **Owner:** Camilo Valencia | **Version:** 1.0 | **Date:** 2025-01-17

---

## Functional Requirements

### FR-100: Motor Control (Core)

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-101 | System shall command a stepper motor to move to a target position (within ±0.1°) | Must | Draft |
| FR-102 | System shall support open-loop (sensorless) control mode | Must | Draft |
| FR-103 | System shall support closed-loop position control with encoder feedback | Must | Draft |
| FR-104 | System shall support trapezoidal velocity profiles | Must | Draft |
| FR-105 | System shall support S-curve (jerk-limited) motion profiles | Must | Draft |
| FR-106 | System shall support higher-order motion profiles (jerk, snap derivatives) | Should | Draft |
| FR-107 | System shall support feedforward compensation | Should | Draft |

### FR-200: Multi-Axis Coordination

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-201 | System shall coordinate 2+ MCU nodes for synchronized motion (≤1ms) | Must | Draft |
| FR-202 | System shall support linear interpolation across multiple axes | Must | Draft |
| FR-203 | System shall support spline interpolation for smooth curved paths | Should | Draft |
| FR-204 | System shall support at least 6 MCU nodes simultaneously | Should | Draft |
| FR-205 | System shall support up to 12 MCU nodes | Could | Draft |
| FR-210 | Each MCU node shall have unique address (1-254, 0 = broadcast) | Must | Draft |
| FR-211 | Host shall send broadcast commands to all nodes simultaneously | Must | Draft |
| FR-212 | MCU nodes shall synchronize motion start via GPIO sync line | Must | Draft |
| FR-213 | System shall support RS-485 or CAN FD daisy-chain (v1.5+) | Should | Draft |

### FR-300: Driver Support

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-301 | System shall support TMC2208 drivers via UART + Step/Dir/En signals | Must | Draft |
| FR-302 | System shall support TMC2209 drivers via UART + Step/Dir/En | Should | Draft |
| FR-303 | System shall support generic step/dir drivers | Should | Draft |
| FR-304 | System shall read driver diagnostics (current, temperature, errors) | Should | Draft |

### FR-400: Sensor Support

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-401 | System shall read AS5600 magnetic encoder position (12-bit resolution) | Must | Draft |
| FR-402 | System shall support limit switches for homing (open-loop mode only) | Must | Draft |
| FR-403 | System shall recognize that absolute encoders eliminate homing need | Should | Draft |
| FR-404 | System shall support environmental sensors (BME680) | Should | Draft |
| FR-405 | System shall support IMU for orientation feedback | Should | Draft |
| FR-406 | System shall support generic hall/magnetic encoders | Could | Draft |

### FR-500: Communication

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-501 | MCU shall communicate with host via USB serial | Must | Draft |
| FR-502 | Protocol shall support command/response pattern | Must | Draft |
| FR-503 | Protocol shall support streaming telemetry (≥100 Hz internal) | Must | Draft |
| FR-504 | Protocol shall include checksums/error detection | Should | Draft |
| FR-505 | System shall support WiFi/MQTT communication | Could | Draft |

### FR-600: GUI Control

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-601 | GUI shall display current position of all axes (10-20 Hz update) | Must | Draft |
| FR-602 | GUI shall provide jog controls (manual move) | Must | Draft |
| FR-603 | GUI shall allow target position entry | Must | Draft |
| FR-604 | GUI shall allow velocity target entry for continuous rotation | Must | Draft |
| FR-605 | GUI shall display motor/driver status (temp, current) | Should | Draft |
| FR-606 | GUI shall provide homing controls | Should | Draft |
| FR-607 | GUI shall visualize motion paths (2D/3D preview) | Could | Draft |
| FR-608 | GUI shall display digital twin using URDF robot definition | Could | Draft |

### FR-700: Safety

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-701 | System shall bring motors to safe position/state on communication timeout | Must | Draft |
| FR-702 | System shall respect limit switch boundaries | Must | Draft |
| FR-703 | System shall monitor sensor health and trigger recovery on disconnect | Must | Draft |
| FR-704 | System shall monitor driver temperature and warn/stop on overheat | Should | Draft |
| FR-705 | System shall detect overcurrent conditions | Should | Draft |
| FR-706 | System shall provide configurable safe position per axis | Should | Draft |
| FR-707 | System shall support software position limits | Could | Draft |

### FR-800: Demo Application

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| FR-801 | Pick-and-place demo shall execute complete pick-place cycle | Must | Draft |
| FR-802 | Demo shall include gripper control | Must | Draft |
| FR-803 | Demo shall be configurable for different positions | Should | Draft |
| FR-804 | Demo shall support generic end effectors (pneumatic, valves, etc.) | Could | Draft |

---

## Non-Functional Requirements

### NFR-100: Performance

| ID | Requirement | Target | Priority | Status |
|----|-------------|--------|----------|--------|
| NFR-101 | Maximum step generation rate | ≥100 kHz | Must | Draft |
| NFR-102 | Position control loop rate | ≥100 Hz | Must | Draft |
| NFR-103 | Velocity control loop rate | ≥500 Hz | Must | Draft |
| NFR-104 | Torque control loop rate | ≥2.5 kHz | Should | Draft |
| NFR-105 | Internal safety telemetry rate | ≥100 Hz | Must | Draft |
| NFR-106 | GUI display update rate | 10-20 Hz | Must | Draft |

### NFR-200: Accuracy

| ID | Requirement | Target | Priority | Status |
|----|-------------|--------|----------|--------|
| NFR-201 | Position accuracy (closed-loop, minimum) | ±0.1° | Must | Draft |
| NFR-202 | Position accuracy (closed-loop, preferred) | ±0.05° | Should | Draft |
| NFR-203 | Multi-axis synchronization | ≤1 ms | Must | Draft |

### NFR-300: Reliability

| ID | Requirement | Target | Priority | Status |
|----|-------------|--------|----------|--------|
| NFR-301 | Communication error rate | <0.1% packet loss | Must | Draft |
| NFR-302 | Watchdog timeout | ≤500 ms | Must | Draft |
| NFR-303 | Continuous operation | ≥24 hours without reset | Should | Draft |

### NFR-400: Usability

| ID | Requirement | Target | Priority | Status |
|----|-------------|--------|----------|--------|
| NFR-401 | First-time setup | <30 minutes with docs | Must | Draft |
| NFR-402 | GUI learning curve | <5 minutes for basic use | Must | Draft |
| NFR-403 | Configuration via file | YAML/JSON supported | Should | Draft |
| NFR-404 | URDF support for robot definition | Load robot model from URDF file | Should | Draft |

### NFR-500: Maintainability

| ID | Requirement | Target | Priority | Status |
|----|-------------|--------|----------|--------|
| NFR-501 | Code documentation | All public APIs documented | Must | Draft |
| NFR-502 | Unit test coverage | ≥80% for core modules | Must | Draft |
| NFR-503 | Modular architecture | Add new driver type in <1 day | Should | Draft |

### NFR-600: Platform Support

| ID | Requirement | Target | Priority | Status |
|----|-------------|--------|----------|--------|
| NFR-601 | MCU platform | ESP32-S3 | Must | Draft |
| NFR-602 | Host platforms | Windows, Linux (PC), RPi 5 | Must | Draft |
| NFR-603 | Python version | ≥3.10 | Must | Draft |
| NFR-604 | macOS host support | Works but not primary | Should | Draft |

---

## Requirement Summary

| Category | Must | Should | Could | Total |
|----------|------|--------|-------|-------|
| Motor Control | 5 | 2 | 0 | 7 |
| Multi-Axis/Network | 5 | 3 | 1 | 9 |
| Drivers | 1 | 3 | 0 | 4 |
| Sensors | 2 | 3 | 1 | 6 |
| Communication | 3 | 1 | 1 | 5 |
| GUI | 4 | 2 | 2 | 8 |
| Safety | 3 | 3 | 1 | 7 |
| Demo | 2 | 1 | 1 | 4 |
| **Functional Total** | **25** | **18** | **7** | **50** |
| **Non-Functional** | **12** | **8** | **0** | **20** |
| **Grand Total** | **37** | **26** | **7** | **70** |

---

## Priority Key (MoSCoW)

| Priority | Meaning |
|----------|---------|
| **Must** | Critical, non-negotiable — project fails without it |
| **Should** | Important, but workarounds exist if needed |
| **Could** | Nice to have, if time and resources permit |
| **Won't** | Explicitly out of scope for this version |

---

## Status Key

| Status | Meaning |
|--------|---------|
| Draft | Not yet validated |
| Approved | Validated and committed |
| Implemented | Code complete |
| Tested | Verified working |

---

## User Stories

### Robot Builder

- As a **robot builder**, I want to **move motors to precise positions** so that **I can build accurate machines**
- As a **robot builder**, I want to **coordinate multiple axes smoothly** so that **I can create complex, curved motion paths**
- As a **robot builder**, I want to **see real-time position feedback** so that **I know where my robot is**
- As a **robot builder**, I want to **define my robot with URDF** so that **I can reuse ROS-compatible definitions**
- As a **robot builder**, I want to **set continuous rotation speed** so that **I can use motors as spindles/conveyors**

### Developer

- As a **developer**, I want to **add support for new drivers** so that **I can use different hardware**
- As a **developer**, I want to **add support for new end effectors** so that **I can control grippers, pneumatics, etc.**

### Hobbyist

- As a **hobbyist**, I want to **use a simple GUI** so that **I don't need to write code for basic use**
- As a **hobbyist**, I want to **see a 3D model of my robot moving** so that **I can visualize what's happening**

### Safety-Conscious User

- As a **safety-conscious user**, I want to **know the robot will go to a safe position on failure** so that **my robot won't crash and break things**

---

## Out of Scope (v1.0)

- **AC servos** — Industrial, out of DIY scope
- **Quadruped robot** — Too complex for v1.0
- **ROS2 integration** — Deferred to after standalone is mature
- **CAN bus communication** — Future industrial expansion
- **Multi-ESP32 via ESP-NOW** — Future wireless chaining
- **DC motors** — After stepper core is stable
- **BLDC motors** — After DC support

---

## Dependencies

| Dependency | Type | Required For | Version/Notes |
|------------|------|--------------|---------------|
| PlatformIO | Tool | Firmware build | Latest stable |
| ESP32-S3 | Hardware | MCU target | DevKitC-1 or similar |
| TMC2208 | Hardware | Stepper driver | Primary driver support |
| AS5600 | Hardware | Position feedback | 12-bit magnetic encoder |
| Python | Runtime | Host software | ≥3.10 |
| pyserial | Library | MCU communication | ≥3.5 |
| numpy | Library | Math operations | ≥1.24 |

---

## Assumptions

1. **Hardware available:** User has ESP32-S3, TMC2208, and AS5600 (or compatible)
2. **Python installed:** Python 3.10+ available on host system
3. **USB connection:** Reliable USB-serial connection between host and MCU
4. **Power supply:** Adequate power supply for motors and drivers
5. **PlatformIO:** User can install and use PlatformIO for firmware builds

---

## Traceability

| Requirement | Supports Goal |
|-------------|---------------|
| FR-101 to FR-107 | Goal 2: Precision Motion Control |
| FR-201 to FR-205 | Goal 4: Multi-Axis Coordination |
| FR-301 to FR-304 | Goal 1: Universal Driver Support |
| FR-401 to FR-406 | Goal 3: Flexible Control Modes |
| FR-501 to FR-505 | Goal 5: Full Stack |
| FR-601 to FR-608 | Goal 6: DIY Focus |
| FR-701 to FR-707 | Goal 7: Safety & Monitoring |

---

## Related Documents

- [DISCOVERY.md](DISCOVERY.md) — Previous phase (vision, goals)
- [ARCHITECTURE.md](ARCHITECTURE.md) — Next phase (system design)
- [SDD Process](../../.github/processes/spec-driven-development.md) — Workflow reference

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-01-17 | Camilo Valencia | Initial requirements from SDD phase |

---

*Approved: 2025-01-17*
