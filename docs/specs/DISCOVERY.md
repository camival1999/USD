# USD Discovery Specification

> **Owner:** Camilo Valencia | **Version:** 1.0 | **Date:** 2025-01-17

---

## Vision Statement

> A full-stack, DIY-friendly motion control system for stepper, DC, and BLDC motors that bridges hobbyist hardware (ESP32 + TMC drivers) with high-level software (Python/PC/RPi), enabling precise multi-axis coordination for personal robotics projects.

---

## Core Goals

| # | Goal | Description |
|---|------|-------------|
| 1 | **Universal Driver Support** | TMC2209, TMC2208, extensible to other commercial drivers |
| 2 | **Precision Motion Control** | High-order profiles: torque → acceleration → velocity → position |
| 3 | **Flexible Control Modes** | Open-loop, closed-loop, nested PID, feedforward |
| 4 | **Multi-Axis Coordination** | Synchronized motion, up to 12 axes max |
| 5 | **Full Stack** | MCU firmware (C++/PlatformIO) + host software (Python) |
| 6 | **DIY Focus** | Hobbyist components, excellent software/control to compensate |
| 7 | **Safety & Monitoring** | Comms health, execution timing, device alerts (thermal, overcurrent, etc.) |

---

## v1.0 Scope (MoSCoW)

| Priority | Features |
|----------|----------|
| **Must** | Single-axis precision positioning |
| **Must** | Multi-axis coordinated motion |
| **Must** | Pick-and-place robot demo |
| **Must** | GUI-based manual control |
| **Could** | 6-DOF robot arm control |
| **Could** | Headless/embedded operation |
| **Won't (v1.0)** | Quadruped robot |
| **Won't (v1.0)** | ROS2 integration |

---

## Hardware Platform

### Primary Configuration

| Component | Choice | Notes |
|-----------|--------|-------|
| **MCU** | ESP32-S3 | Primary target for v1.0 |
| **Host** | PC or Raspberry Pi 5 | Interchangeable for standalone operation |
| **Real-time** | As close to hard real-time as possible on ESP32 |
| **Build System** | PlatformIO | For all MCU firmware |

### Future MCU Support (Post-v1.0)

| MCU | Priority | Challenge |
|-----|----------|-----------|
| STM32 | Medium | Cross-platform abstraction needed |
| ATmega | Low | Resource constrained |

### Communication Roadmap

| Priority | Interface | Notes |
|----------|-----------|-------|
| **v1.0 Must** | UART | Primary MCU↔driver communication |
| **v1.0 Must** | USB | MCU↔PC/RPi connection |
| **v1.0 Must** | SPI | Fast MCU↔driver/sensor |
| **v1.0 Should** | I2C | Sensors, displays |
| **Future** | CAN | Industrial expansion |
| **Future** | MQTT/WiFi | Wireless control |
| **Future** | ESP-NOW | Multi-ESP32 chaining |

---

## Step Generation Architecture

| Aspect | Design |
|--------|--------|
| **Pulse Generation** | MCU generates step pulses for TMC drivers |
| **Digital Drivers** | Some drivers (e.g., TMC UART mode) handle steps internally—architecture must stay flexible |
| **Host Role** | Motion planning, trajectory generation, supervisory control |
| **MCU Role** | Real-time step generation, sensor reading, safety monitoring |

---

## Performance Targets

| Metric | Target | Rationale |
|--------|--------|-----------|
| **Max step rate** | 100 kHz | 2000 RPM × 200 steps × 32 µsteps ≈ 64 kHz, headroom to 100 kHz |
| **Position accuracy** | ±0.05° preferred, ±0.1° acceptable | |
| **Max axes** | 12 | Quadruped spec, but v1.0 focuses on ≤6 |

### Control Loop Hierarchy

| Loop | Frequency | Depends On |
|------|-----------|------------|
| **Position** | 100 Hz – 5 kHz | Host hardware capability |
| **Velocity** | 500 Hz – 25 kHz | ≥5× position rate |
| **Torque/Accel** | 2.5 kHz – 125 kHz | ≥5× velocity rate |

> **Note:** Inner loops (torque/velocity) likely run on MCU; outer loops (position) may run on host depending on hardware.

---

## Sensors & Feedback

### Position/Velocity Feedback (v1.0)

| Sensor | Priority | Notes |
|--------|----------|-------|
| **AS5600** | Must | Primary magnetic encoder, available |
| **Other hall/magnetic encoders** | Should | Common hobby-grade sensors |
| **Optical encoders** | Won't | Not suited for high-performance DIY |
| **Mechanical encoders** | Won't | Same reason |

### Force/Torque Feedback

| Method | Notes |
|--------|-------|
| **Current sensing** | Via TMC driver current reports or shunt sensing |
| **No dedicated force sensors** | Keep mechanical simple |

### Environmental & Safety Sensors

| Sensor Type | Examples | Priority |
|-------------|----------|----------|
| **Temperature/Environment** | BME680 | Should |
| **Limit/homing switches** | Mechanical, magnetic reed | Must |
| **Distance sensors** | Ultrasonic, IR | Could |
| **IMU** | End-effector/joint feedback | Should |

---

## Constraints & Exclusions

### Out of Scope

| Excluded | Reason |
|----------|--------|
| **AC servos** | Industrial, out of DIY scope |
| **Product deadlines** | Hobby pacing |

### Future Expansion (Post-v1.0)

| Feature | When |
|---------|------|
| DC motors | After stepper core is stable |
| BLDC motors | After DC support |
| ROS2 integration | After standalone is mature |
| CAN bus | Industrial expansion |
| Multi-ESP32 via ESP-NOW | Wireless chaining |

---

## Project Classification

| Field | Value |
|-------|-------|
| **Project Type** | Full-stack motion control (firmware + software) |
| **Primary MCU** | ESP32-S3 (via PlatformIO) |
| **Primary Host** | PC or Raspberry Pi 5 |
| **Motor Types (v1.0)** | Stepper only |
| **Max Axes** | 12 (focus on ≤6 for v1.0) |
| **Control** | Nested loops: position (100Hz-5kHz) → velocity → torque |
| **Real-time** | Hard real-time goal on MCU |
| **Pacing** | Hobby/personal project |

---

## Success Criteria

| Criterion | Measure |
|-----------|---------|
| **Single-axis works** | Motor moves to commanded position within ±0.1° |
| **Multi-axis works** | 3+ axes move synchronously for coordinated path |
| **Pick-and-place demo** | Complete pick-up and place cycle runs autonomously |
| **GUI functional** | User can jog motors, set positions, monitor status |
| **Safety active** | Overtemp/overcurrent triggers protective stop |

---

## Open Questions

| # | Question | Status |
|---|----------|--------|
| 1 | How to abstract MCU differences for future STM32 support? | Deferred to v2.0 |
| 2 | Best protocol for host↔MCU communication (custom binary vs protobuf vs JSON)? | TBD in Architecture |
| 3 | GUI framework choice (PyQt vs Tkinter vs web-based)? | TBD in Architecture |

---

## Related Documents

- [REQUIREMENTS.md](REQUIREMENTS.md) — Next phase
- [ARCHITECTURE.md](ARCHITECTURE.md) — System design
- [SDD Process](../../.github/processes/spec-driven-development.md) — Workflow reference

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-01-17 | Camilo Valencia | Initial discovery from SDD kickoff |

---

*Approved: 2025-01-17*
