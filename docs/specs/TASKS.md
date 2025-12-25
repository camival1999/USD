# Tasks Specification

> **Project:** Ultimate Stepper Driver (USD) | **Version:** 0.1 | **Last Updated:** 2025-01-17

---

## Development Strategy

**Sequential Development Order:**
- Firmware first (P0-P5, P8) — leverages existing architecture
- Host software second (P6) — enables testing without hardware
- GUI last (P7) — focused styling and UX polish
- Documentation throughout (P9)

**Test Strategy:**
- 80% test coverage target for all phases
- Unit tests with mocks for hardware not available
- Simulated encoder/limits until real hardware arrives
- Hardware available: TMC2208 ✓, ESP32-S3 ✓, steppers ✓
- Hardware pending: AS5600 (weeks), PSU, real limit switches

---

## Phase Overview

```
P0 ──> P1 ──> P2 ──> P3 ──> P4 ──> P5 ──> P8 ──> P6 ──> P7 ──> P9
 │      │      │      │      │      │      │      │      │      │
 └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
           FIRMWARE PHASES                 │    HOST PHASES
                                           │
                                      Safety before
                                      host integration
```

| Phase | Name | Focus | Est. Hours | Test Strategy |
|-------|------|-------|------------|---------------|
| **P0** | Foundation | Build system, FreeRTOS skeleton | 16h | Build verification |
| **P1** | Core Motion | Open-loop stepping, profiles | 34h | Unit + mock hardware |
| **P2** | Driver Integration | TMC2208/2209, UART | 26h | Real TMC + steppers |
| **P3** | Sensor Integration | AS5600, limits | 22h | Simulated initially |
| **P4** | Closed-Loop | PID, position/velocity loops | 25h | Simulated encoder |
| **P5** | Communication | COBS, USB protocol | 27h | Loopback + mock |
| **P8** | Safety & Polish | Faults, limits, watchdog | 21h | Fault injection |
| **P6** | Host Software | Python package | 23h | 80% pytest coverage |
| **P7** | GUI | PyQt6 interface | 26h | Manual + widget tests |
| **P9** | Demos & Docs | Examples, guides | 14h | End-to-end validation |

**Total Estimated: ~234 hours for v1.0**

---

## Milestones

| Milestone | Phase Complete | Key Deliverable | Target |
|-----------|----------------|-----------------|--------|
| **M1: Motor Spins** | P2 | TMC2208 drives stepper via ESP32 | — |
| **M2: Protocol Works** | P5 | Host sends command, MCU responds | — |
| **M3: Closed Loop** | P4 + P3 | Position control with encoder | — |
| **M4: Python Control** | P6 | Move motor from Python script | — |
| **M5: GUI Ready** | P7 | Full GUI controls motor | — |
| **M6: v1.0 Release** | P9 | Complete package with docs | — |

---

## Phase P0: Foundation (16h)

**Goal:** Establish build system, FreeRTOS task structure, test frameworks

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P0.1 | PlatformIO setup | ESP32-S3 target, USB-CDC, 240MHz | — | 2h | ⬜ |
| P0.2 | FreeRTOS task skeleton | 5 tasks (comm, motion, safety, sensor, sync) | P0.1 | 4h | ⬜ |
| P0.3 | Python package structure | pyproject.toml, usd/, gui/ folders | — | 2h | ⬜ |
| P0.4 | Protocol definitions | common/protocol/ with .h + .py mirrors | — | 3h | ⬜ |
| P0.5 | Test frameworks | PlatformIO native + pytest setup | P0.1, P0.3 | 3h | ⬜ |
| P0.6 | Debug logging | USB-CDC printf wrapper | P0.2 | 2h | ⬜ |

**Definition of Done:**
- [ ] All tasks compile without warnings
- [ ] FreeRTOS tasks start and print to console
- [ ] Python package imports successfully
- [ ] Test frameworks run (0 tests OK)

---

## Phase P1: Core Motion (34h)

**Goal:** Open-loop motor control with trapezoidal and S-curve profiles

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P1.1 | IDriver interface | Abstract base: step/dir/enable methods | P0.2 | 2h | ⬜ |
| P1.2 | GenericStepDirDriver | GPIO-based implementation | P1.1 | 4h | ⬜ |
| P1.3 | MCPWM step generator | Hardware PWM timing for steps | P0.2 | 6h | ⬜ |
| P1.4 | Timer interrupt fallback | ISR-based stepping when MCPWM unavailable | P1.3 | 4h | ⬜ |
| P1.5 | MotionController | State machine, open-loop move commands | P1.2, P1.3 | 6h | ⬜ |
| P1.6 | TrajectoryInterpolator | Trapezoidal profile (accel/cruise/decel) | P1.5 | 6h | ⬜ |
| P1.7 | S-curve profile | Jerk-limited smooth acceleration | P1.6 | 4h | ⬜ |
| P1.8 | Motion unit tests | 80% coverage for P1 modules | P1.1-P1.7 | 4h | ⬜ |

**Definition of Done:**
- [ ] Open-loop move executes on real stepper
- [ ] Trapezoidal profile: visible accel/cruise/decel
- [ ] S-curve profile: smooth jerk-limited motion
- [ ] 80% test coverage

---

## Phase P2: Driver Integration (26h)

**Goal:** TMC2208/2209 UART configuration, microstepping, current control

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P2.1 | TMC UART interface | 1-wire bidirectional using ESP32 UART | P0.2 | 4h | ⬜ |
| P2.2 | TMC2208Driver | IDriver impl with UART config | P2.1, P1.1 | 6h | ⬜ |
| P2.3 | Microstepping config | 1/2/4/8/16 via UART registers | P2.2 | 2h | ⬜ |
| P2.4 | Current limiting | IRUN/IHOLD configuration via UART | P2.2 | 3h | ⬜ |
| P2.5 | TMC2209Driver | stealthChop/spreadCycle mode switching | P2.2 | 4h | ⬜ |
| P2.6 | StallGuard support | Sensorless stall detection (TMC2209) | P2.5 | 4h | ⬜ |
| P2.7 | Driver factory | Runtime driver selection from config | P2.2, P2.5 | 3h | ⬜ |

**Definition of Done:**
- [ ] TMC2208 runs motor with UART-configured microstepping
- [ ] Current limit verified with measurement
- [ ] StallGuard detects motor stall (TMC2209)
- [ ] Factory selects correct driver from config

---

## Phase P3: Sensor Integration (22h)

**Goal:** Encoder position reading, limit switch handling, homing

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P3.1 | I2C abstraction | Wire wrapper with error handling | P0.2 | 3h | ⬜ |
| P3.2 | ISensor/IEncoder | Abstract sensor interfaces | P0.2 | 2h | ⬜ |
| P3.3 | AS5600Encoder | I2C driver, 12-bit position reading | P3.1, P3.2 | 4h | ⬜ |
| P3.4 | Multi-turn tracking | Software accumulation across rollovers | P3.3 | 3h | ⬜ |
| P3.5 | ILimitSwitch | Abstract limit switch interface | P0.2 | 1h | ⬜ |
| P3.6 | LimitSwitch impl | GPIO with hardware debounce | P3.5 | 3h | ⬜ |
| P3.7 | SensorTask | Periodic polling, interrupt handling | P3.3, P3.6 | 4h | ⬜ |
| P3.8 | Homing sequence | Home to limit switch or StallGuard | P3.6, P1.5 | 4h | ⬜ |

**Definition of Done:**
- [ ] Encoder reads position (simulated OK initially)
- [ ] Multi-turn tracking handles rollover correctly
- [ ] Limit switches trigger (simulated with normal switches)
- [ ] Homing sequence completes

---

## Phase P4: Closed-Loop Control (25h)

**Goal:** PID position and velocity control loops on MCU

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P4.1 | PIDController class | P, I, D terms with output clamping | P0.2 | 4h | ⬜ |
| P4.2 | Anti-windup | Integral clamping on output saturation | P4.1 | 2h | ⬜ |
| P4.3 | Derivative filter | Low-pass filter on D term | P4.1 | 2h | ⬜ |
| P4.4 | Position loop | Encoder feedback → velocity command | P4.1, P3.3 | 6h | ⬜ |
| P4.5 | Velocity loop | Nested under position loop | P4.4 | 4h | ⬜ |
| P4.6 | Tuning interface | Get/set PID gains at runtime | P4.1 | 3h | ⬜ |
| P4.7 | Mode switching | Seamless open-loop ↔ closed-loop | P4.4, P1.5 | 2h | ⬜ |
| P4.8 | Limits enforcement | Position and velocity software limits | P4.4 | 2h | ⬜ |

**Definition of Done:**
- [ ] Closed-loop position control works (simulated encoder)
- [ ] PID gains adjustable at runtime
- [ ] Smooth mode switching
- [ ] 80% test coverage

---

## Phase P5: Communication (27h)

**Goal:** COBS-framed binary protocol over USB-CDC

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P5.1 | COBS encoder (C++) | Byte stuffing, 0x00 frame delimiters | P0.2 | 3h | ⬜ |
| P5.2 | COBS decoder (C++) | Frame extraction from stream | P5.1 | 2h | ⬜ |
| P5.3 | CRC-16-CCITT | Polynomial 0x1021, initial 0xFFFF | P0.2 | 2h | ⬜ |
| P5.4 | Packet structures | Protocol.h with all message types | P0.4 | 3h | ⬜ |
| P5.5 | UsbSerialComm | IHostComm implementation for USB-CDC | P5.1, P5.3 | 4h | ⬜ |
| P5.6 | CommTask | Receive loop, CRC check, dispatch | P5.5 | 4h | ⬜ |
| P5.7 | Command handlers | Move, stop, home, set PID, get status | P5.6, P1.5, P4.6 | 6h | ⬜ |
| P5.8 | Response/event sending | ACK, NAK, STATUS, FAULT packets | P5.6 | 3h | ⬜ |

**Definition of Done:**
- [ ] Host sends command, MCU responds with ACK
- [ ] Invalid packets rejected with NAK
- [ ] All command types functional
- [ ] 80% test coverage

---

## Phase P8: Safety & Polish (21h)

**Goal:** Fault handling, watchdog, safe stop behavior

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P8.1 | SafetyMonitor | Continuous limit switch monitoring | P3.6, P0.2 | 3h | ⬜ |
| P8.2 | FaultHandler | State machine: OK → FAULT → RECOVERY | P8.1 | 4h | ⬜ |
| P8.3 | Watchdog integration | ESP32 TWDT, per-task checks | P0.2 | 2h | ⬜ |
| P8.4 | Safe stop behavior | Controlled deceleration on fault | P8.2, P1.5 | 3h | ⬜ |
| P8.5 | Position persistence | Store last position on fault | P8.2, P3.3 | 2h | ⬜ |
| P8.6 | Fault clear sequence | Explicit clear + optional re-home | P8.2 | 3h | ⬜ |
| P8.7 | Safety integration tests | Fault scenarios, edge cases | P8.1-P8.6 | 4h | ⬜ |

**Definition of Done:**
- [ ] Limit hit → controlled stop → FAULT state
- [ ] Watchdog timeout → safe shutdown
- [ ] Position recorded on fault
- [ ] Recovery sequence works
- [ ] 80% test coverage

---

## Phase P6: Host Software (23h)

**Goal:** Python package for MCU communication and trajectory generation

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P6.1 | COBS codec (Python) | Encode/decode matching firmware | P0.3 | 2h | ⬜ |
| P6.2 | CRC-16 (Python) | Match firmware CRC implementation | P0.3 | 1h | ⬜ |
| P6.3 | protocol.py | Packet classes, serialize/deserialize | P6.1, P6.2 | 4h | ⬜ |
| P6.4 | transport.py | Serial wrapper with timeout handling | P0.3 | 3h | ⬜ |
| P6.5 | usd.core.trajectory | Trapezoidal/S-curve generation | P0.3 | 4h | ⬜ |
| P6.6 | usd.core.units | mm/deg ↔ m/rad conversion utilities | P0.3 | 2h | ⬜ |
| P6.7 | MotorController API | High-level: connect, move, home, status | P6.3, P6.4 | 4h | ⬜ |
| P6.8 | Test suite | 80% pytest coverage for usd package | P6.1-P6.7 | 3h | ⬜ |

**Definition of Done:**
- [ ] Python controls motor via USB
- [ ] Protocol matches firmware exactly
- [ ] Trajectory generation works
- [ ] 80% test coverage

---

## Phase P7: GUI (26h)

**Goal:** Polished PyQt6 interface for motor control and tuning

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P7.1 | main_window.ui | Qt Designer layout for main window | P0.3 | 4h | ⬜ |
| P7.2 | MainController | App entry, connection management | P7.1, P6.7 | 4h | ⬜ |
| P7.3 | Position display widget | Real-time position readout | P7.2 | 3h | ⬜ |
| P7.4 | Velocity gauge widget | Analog-style velocity indicator | P7.3 | 3h | ⬜ |
| P7.5 | Jog controls | +/- buttons, keyboard shortcuts | P7.2 | 3h | ⬜ |
| P7.6 | Velocity mode | Set target velocity for jogging | P7.5 | 2h | ⬜ |
| P7.7 | tuning_dialog.ui | PID gain sliders in Qt Designer | P7.2 | 3h | ⬜ |
| P7.8 | Position plot | Real-time chart with pyqtgraph | P7.3 | 4h | ⬜ |
| P7.9 | QSS stylesheet | Consistent, polished visual styling | P7.1 | 3h | ⬜ |

**Definition of Done:**
- [ ] GUI connects to motor
- [ ] Jog controls work smoothly
- [ ] Position plot updates in real-time
- [ ] Tuning dialog adjusts PID gains
- [ ] Polished, consistent appearance

---

## Phase P9: Demos & Documentation (14h)

**Goal:** Working examples and user documentation

| ID | Task | Description | Deps | Est. | Status |
|----|------|-------------|------|------|--------|
| P9.1 | single_axis demo | Complete working example project | All | 4h | ⬜ |
| P9.2 | getting-started guide | Installation and setup instructions | P9.1 | 3h | ⬜ |
| P9.3 | wiring guide | TMC2208 + AS5600 + ESP32 diagrams | P2.2, P3.3 | 3h | ⬜ |
| P9.4 | test_connection.py | Script to verify MCU communication | P6.4 | 2h | ⬜ |
| P9.5 | README update | Project overview, quick start | P9.2 | 2h | ⬜ |

**Definition of Done:**
- [ ] New user can follow guide to running demo
- [ ] Wiring diagram accurate and clear
- [ ] README provides complete overview

---

## Test Coverage Matrix

| Phase | Coverage Target | Test Type | Mock Strategy |
|-------|-----------------|-----------|---------------|
| P0 | — | Build verification | — |
| P1 | 80% | Unit | Mock step generator timing |
| P2 | 70%* | Unit + Integration | Real TMC where possible |
| P3 | 80% | Unit | Simulated encoder/limits |
| P4 | 80% | Unit | Simulated plant model |
| P5 | 80% | Unit | Loopback packets |
| P6 | 80% | pytest | Mock serial, mock MCU |
| P7 | 60%* | Manual + widget | GUI hard to unit test |
| P8 | 80% | Unit | Fault injection |

*Hardware-dependent phases have adjusted targets

---

## Risk Areas

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **MCPWM complexity** | Medium | High | Timer fallback ready (P1.4), early testing |
| **TMC UART timing** | Medium | Medium | Use ESP32 UART peripheral, logic analyzer |
| **AS5600 multi-turn overflow** | Low | Medium | Careful accumulation, edge case tests |
| **FreeRTOS task starvation** | Low | High | Priority tuning, watchdog (P8.3) |
| **USB latency spikes** | Medium | Low | Queue buffers, async processing |
| **Hardware unavailability** | Medium | Medium | Simulated tests, defer real HW tests |

---

## Dependencies Graph

```
P0.1 ─┬─> P0.2 ─┬─> P1.x ─┬─> P2.x ─> P3.x ─> P4.x ─┬─> P5.x ─> P8.x
      │         │         │                         │
      │         │         └─────────────────────────┘
      │         │
      └─> P0.3 ─┴─> P6.x ─> P7.x
      │
      └─> P0.4 ─> (shared by P5.x and P6.x)
```

---

## Related Documents

- [DISCOVERY.md](DISCOVERY.md) — Vision and goals
- [REQUIREMENTS.md](REQUIREMENTS.md) — What the system must do
- [ARCHITECTURE.md](ARCHITECTURE.md) — System design
- [SDD Process](../../.github/processes/spec-driven-development.md) — Workflow reference
