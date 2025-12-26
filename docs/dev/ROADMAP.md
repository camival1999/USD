# Roadmap

> **Last Updated:** 2025-05-26 | **Maintained by:** Scribe Agent

---

## Current Version: v0.1.0 (P0 Foundation Complete)

---

## Upcoming Milestones

| Milestone | Target Date | Status | Key Features | Notes |
|-----------|-------------|--------|--------------|-------|
| M1: Motor Spins | — | 📋 Planned | TMC2208, MCPWM, open-loop | Phase P2 complete |
| M2: Protocol Works | — | 📋 Planned | COBS, USB-CDC, commands | Phase P5 complete |
| M3: Closed Loop | — | 📋 Planned | PID, AS5600, position control | Phase P3+P4 complete |
| M4: Python Control | — | 📋 Planned | usd package, trajectory gen | Phase P6 complete |
| M5: GUI Ready | — | 📋 Planned | PyQt6, tuning, plots | Phase P7 complete |
| M6: v1.0 Release | — | 📋 Planned | Demos, docs, polish | All phases complete |

### Status Key

| Icon | Status |
|------|--------|
| 📋 | Planned |
| 🟡 | In Progress |
| ✅ | Released |
| ⏸️ | On Hold |

---

## Phase Overview

Total estimated: **~234 hours** for v1.0 | **Completed: ~16 hours**

| Phase | Name | Hours | Status |
|-------|------|-------|--------|
| P0 | Foundation | 16h | ✅ Complete |
| P1 | Core Motion | 34h | 📋 Planned |
| P2 | Driver Integration | 26h | 📋 Planned |
| P3 | Sensor Integration | 22h | 📋 Planned |
| P4 | Closed-Loop Control | 25h | 📋 Planned |
| P5 | Communication | 27h | 📋 Planned |
| P8 | Safety & Polish | 21h | 📋 Planned |
| P6 | Host Software | 23h | 📋 Planned |
| P7 | GUI | 26h | 📋 Planned |
| P9 | Demos & Docs | 14h | 📋 Planned |

See [TASKS.md](../specs/TASKS.md) for detailed task breakdown.

---

## Milestone Details

### M1: Motor Spins

**Target:** — | **Status:** 📋 Planned

**Goals:**
- [ ] ESP32-S3 PlatformIO project running
- [ ] TMC2208 controlled via UART
- [ ] Stepper motor executes open-loop moves
- [ ] Trapezoidal and S-curve profiles working

**Key Deliverables:**
- `firmware/` with working motor control
- TMC2208 microstepping and current limit
- MCPWM step generation

**Phases:** P0, P1, P2

---

### M2: Protocol Works

**Target:** — | **Status:** 📋 Planned

**Goals:**
- [ ] COBS framing implemented
- [ ] CRC-16 integrity checking
- [ ] Host sends commands, MCU responds
- [ ] All command types functional

**Key Deliverables:**
- USB-CDC communication working
- ACK/NAK response flow
- Command handlers (move, stop, home, etc.)

**Phases:** P5

---

### M3: Closed Loop

**Target:** — | **Status:** 📋 Planned

**Goals:**
- [ ] AS5600 encoder reading position
- [ ] Multi-turn tracking working
- [ ] PID position control functional
- [ ] Velocity loop nested under position

**Key Deliverables:**
- Closed-loop position control
- Runtime PID tuning
- Limit switch handling

**Phases:** P3, P4

---

### M4: Python Control

**Target:** — | **Status:** 📋 Planned

**Goals:**
- [ ] Python `usd` package functional
- [ ] Protocol matches firmware exactly
- [ ] High-level MotorController API
- [ ] 80% test coverage

**Key Deliverables:**
- `software/usd/` package
- Move motor from Python script
- Trajectory generation (trapezoidal, S-curve)

**Phases:** P6

---

### M5: GUI Ready

**Target:** — | **Status:** 📋 Planned

**Goals:**
- [ ] PyQt6 main window functional
- [ ] Position/velocity displays working
- [ ] Jog controls responsive
- [ ] Tuning dialog adjusts PID
- [ ] Polished, consistent styling

**Key Deliverables:**
- `software/gui/` application
- Real-time position plot
- QSS stylesheet applied

**Phases:** P7

---

### M6: v1.0 Release

**Target:** — | **Status:** 📋 Planned

**Goals:**
- [ ] Safety features complete (faults, watchdog)
- [ ] single_axis demo project
- [ ] Getting started guide
- [ ] Wiring guide with diagrams
- [ ] README with complete instructions

**Key Deliverables:**
- Complete, documented v1.0 package
- New user can follow guide to working demo

**Phases:** P8, P9

**Target:** YYYY-MM | **Status:** 📋 Planned

**Goals:**
- [ ] [Goal 1]
- [ ] [Goal 2]

**Key Features:**
- [Feature 3] — [Brief description]
- [Feature 4] — [Brief description]

---

## Completed Milestones

| Version | Released | Highlights |
|---------|----------|------------|
| — | — | No releases yet |

---

## Long-Term Vision

<!-- Where is this project headed beyond the immediate roadmap? -->

- [Long-term goal 1]
- [Long-term goal 2]
- [Long-term goal 3]

---

## How to Suggest Changes

1. Open an issue or discuss with the team
2. Changes to roadmap are logged in CHANGELOG.md
3. Scribe will update this file when priorities shift

---

## Related

- [CHANGELOG.md](CHANGELOG.md) — What's been released
- [TASKS.md](../specs/TASKS.md) — Detailed implementation tasks
- [FEATURES/](FEATURES/) — Per-feature status
