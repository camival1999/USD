# Architecture Specification

> **Project:** Ultimate Stepper Driver (USD) | **Version:** 0.1 | **Last Updated:** 2025-01-17

---

## System Overview

### System Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              HOST (PC / RPi5)                               │
├─────────────────────────────────────────────────────────────────────────────┤
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                           Python Application                          │  │
│  │  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────────┐   │  │
│  │  │  usd.comms  │───>│  usd.core   │<───│      usd.gui (PyQt6)    │   │  │
│  │  │  Protocol   │    │  Trajectory │    │  Control │ Tuning │ Sim │   │  │
│  │  │  Handler    │    │  Generator  │    └─────────────────────────┘   │  │
│  │  └──────┬──────┘    └─────────────┘                                   │  │
│  └─────────│─────────────────────────────────────────────────────────────┘  │
│            │ USB-CDC (v1.0) / RS-485 / CAN FD (v1.5+)                       │
└────────────│────────────────────────────────────────────────────────────────┘
             │
═════════════╪═════════════════════════════════════════════════════════════════
             │ COBS-framed Binary + CRC-16
═════════════╪═════════════════════════════════════════════════════════════════
             │
┌────────────┴────────────────────────────────────────────────────────────────┐
│                           MCU (ESP32-S3)                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                        FreeRTOS Task Structure                        │  │
│  │                                                                       │  │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐  │  │
│  │  │  Comm    │  │  Motion  │  │  Safety  │  │  Sensor  │  │  Sync  │  │  │
│  │  │  Task    │  │  Task    │  │  Task    │  │  Task    │  │  Task  │  │  │
│  │  │ (NORMAL) │  │ (HIGH)   │  │(HIGHEST) │  │ (NORMAL) │  │ (HIGH) │  │  │
│  │  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬───┘  │  │
│  │       │             │             │             │             │       │  │
│  │       v             v             v             v             v       │  │
│  │  ┌─────────────────────────────────────────────────────────────────┐  │  │
│  │  │            Shared State (FreeRTOS Queue + Mutex)               │  │  │
│  │  └─────────────────────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        Hardware Abstraction                         │    │
│  │  ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌───────────────────┐│    │
│  │  │ MCPWM/    │  │ TMC UART  │  │ I2C/SPI   │  │  GPIO (Limits,    ││    │
│  │  │ Timer     │  │ Interface │  │ Sensors   │  │  Enable, Sync)    ││    │
│  │  └─────┬─────┘  └─────┬─────┘  └─────┬─────┘  └─────────┬─────────┘│    │
│  └────────│──────────────│──────────────│──────────────────│──────────┘    │
└───────────│──────────────│──────────────│──────────────────│────────────────┘
            │              │              │                  │
════════════╪══════════════╪══════════════╪══════════════════╪════════════════
            v              v              v                  v
     ┌───────────┐  ┌───────────┐  ┌───────────┐     ┌───────────────┐
     │  STEP/DIR │  │  TMC2208/ │  │  AS5600   │     │ Limit Switches│
     │  Signals  │  │  TMC2209  │  │  Encoder  │     │ / GPIO Sync   │
     └───────────┘  └───────────┘  └───────────┘     └───────────────┘
```

### Description

The Ultimate Stepper Driver (USD) is a **full-stack motion control system** targeting hobbyist and DIY robotics applications. It provides real-time motor control on an ESP32-S3 microcontroller with a feature-rich Python host application for trajectory generation, tuning, and visualization.

**Key Architectural Patterns:**

- **Real-Time / Non-Real-Time Split:** All time-critical control loops (position, velocity, torque) run on the MCU. The host handles trajectory planning, UI, and data logging.
- **Hardware Abstraction:** Driver (TMC2208/2209), sensor (AS5600), and communication interfaces are abstracted to enable easy addition of new hardware.
- **COBS-Framed Binary Protocol:** Efficient, robust serial communication with CRC-16 integrity checking.
- **FreeRTOS Task Isolation:** Each responsibility (motion, safety, sensors, comms) runs in dedicated tasks with priority-based scheduling.

**Scope for v1.0:**

- Single-axis stepper control via USB
- TMC2208 (primary) / TMC2209 (secondary) driver support
- AS5600 magnetic encoder feedback
- Trapezoidal and S-curve trajectory profiles
- Open-loop and closed-loop modes
- PyQt6 desktop GUI (functional prototype)

**Future (v1.5+):**

- Multi-axis coordination via RS-485 or CAN FD daisy-chain
- Broadcast synchronization for coordinated motion
- Additional motor types (DC, BLDC)

---

## Firmware Architecture

### Library Structure

```
firmware/
├── platformio.ini
├── src/
│   └── main.cpp                   # Entry point, task creation
├── lib/
│   ├── usd_core/                  # Motion control algorithms
│   │   ├── MotionController.h/cpp
│   │   ├── TrajectoryInterpolator.h/cpp
│   │   ├── PIDController.h/cpp
│   │   └── ControlLoop.h/cpp
│   ├── usd_drivers/               # Motor driver abstractions
│   │   ├── IDriver.h              # Interface
│   │   ├── TMC2208Driver.h/cpp
│   │   ├── TMC2209Driver.h/cpp
│   │   └── GenericStepDirDriver.h/cpp
│   ├── usd_sensors/               # Sensor abstractions
│   │   ├── ISensor.h              # Interface
│   │   ├── AS5600Encoder.h/cpp
│   │   └── LimitSwitch.h/cpp
│   ├── usd_comms/                 # Communication handling
│   │   ├── IHostComm.h            # Interface
│   │   ├── UsbSerialComm.h/cpp
│   │   ├── Rs485Comm.h/cpp        # v1.5+
│   │   ├── CanFdComm.h/cpp        # v1.5+
│   │   ├── COBSCodec.h/cpp
│   │   ├── CRC16.h/cpp
│   │   └── Protocol.h             # Packet definitions
│   └── usd_safety/                # Safety subsystem
│       ├── SafetyMonitor.h/cpp
│       └── FaultHandler.h/cpp
└── test/                          # Unit tests (PlatformIO native)
```

### FreeRTOS Task Structure

| Task | Priority | Rate | Responsibility |
|------|----------|------|----------------|
| **SafetyTask** | HIGHEST (5) | 1 kHz | Monitor limits, watchdog, fault handling |
| **MotionTask** | HIGH (4) | 100 Hz–5 kHz | Position/velocity/torque control loops |
| **SyncTask** | HIGH (4) | On interrupt | GPIO sync line handling |
| **SensorTask** | NORMAL (3) | 1–10 Hz (env) / 2.5 kHz+ (encoder) | Read sensors, update state |
| **CommTask** | NORMAL (3) | On data | Parse packets, send responses |
| **IdleTask** | LOWEST (0) | — | System idle, power management |

### Control Loop Architecture

All control loops execute on the MCU for deterministic timing:

```
                    Host                           MCU
              ┌─────────────┐             ┌───────────────────────┐
              │  Trajectory │             │                       │
              │  Generator  │   Target    │  ┌─────────────────┐  │
              │             │ ──────────> │  │ Position Loop   │  │
              │ (waypoints, │  Position   │  │ 100 Hz – 5 kHz  │  │
              │  profiles)  │             │  └────────┬────────┘  │
              └─────────────┘             │           │           │
                                          │           v           │
                                          │  ┌─────────────────┐  │
                                          │  │ Velocity Loop   │  │
                                          │  │ 500 Hz – 25 kHz │  │
                                          │  └────────┬────────┘  │
                                          │           │           │
                                          │           v           │
                                          │  ┌─────────────────┐  │
                                          │  │ Torque/Current  │  │
                                          │  │ (if closed-loop)│  │
                                          │  │ 2.5 kHz+        │  │
                                          │  └────────┬────────┘  │
                                          │           │           │
                                          │           v           │
                                          │  ┌─────────────────┐  │
                                          │  │ Step Generator  │  │
                                          │  │ MCPWM / Timer   │  │
                                          │  └─────────────────┘  │
                                          └───────────────────────┘
```

### Step Generation

| Method | Timing | Max Frequency | Use Case |
|--------|--------|---------------|----------|
| **MCPWM** (primary) | Hardware | ~500 kHz | High-speed, jitter-free stepping |
| **Timer Interrupt** (fallback) | ISR | ~100 kHz | If MCPWM channels exhausted |
| **GPIO Bit-Bang** (last resort) | Task | ~20 kHz | Debug/testing only |

---

## Software Architecture

### Python Package Structure

```
software/
├── pyproject.toml
├── usd/
│   ├── __init__.py
│   ├── core/                      # Motion primitives
│   │   ├── __init__.py
│   │   ├── trajectory.py          # Trajectory generation
│   │   ├── kinematics.py          # Forward/inverse kinematics
│   │   └── units.py               # Unit conversion (mm/deg ↔ SI)
│   ├── comms/                     # Communication layer
│   │   ├── __init__.py
│   │   ├── protocol.py            # Packet encoding/decoding
│   │   ├── cobs.py                # COBS framing
│   │   ├── crc16.py               # CRC-16-CCITT
│   │   └── transport.py           # Serial/USB abstraction
│   ├── drivers/                   # Driver configuration mirrors
│   │   ├── __init__.py
│   │   ├── tmc2208.py
│   │   └── tmc2209.py
│   └── utils/                     # Shared utilities
│       ├── __init__.py
│       └── logging.py
├── gui/
│   ├── __init__.py
│   ├── app.py                     # Main application entry
│   ├── windows/                   # Qt Designer .ui files
│   │   ├── main_window.ui
│   │   ├── tuning_dialog.ui
│   │   └── simulation_widget.ui
│   ├── widgets/                   # Custom widgets
│   │   ├── position_plot.py
│   │   ├── velocity_gauge.py
│   │   └── motor_status.py
│   ├── controllers/               # UI logic (MVC pattern)
│   │   ├── main_controller.py
│   │   ├── tuning_controller.py
│   │   └── simulation_controller.py
│   └── resources/
│       ├── styles.qss             # Qt stylesheet
│       └── icons/
└── tests/
    ├── test_trajectory.py
    ├── test_protocol.py
    └── test_cobs.py
```

### GUI Workflow

**Development Approach:** Prototype first, beautify later.

1. **Layout:** Design layouts in **Qt Designer** (`.ui` files)
2. **Logic:** Implement controllers in Python (separate from UI)
3. **Styling:** Apply **QSS stylesheet** for consistent look
4. **URDF Support (Could):** Optional digital twin visualization

### Unit Handling

| Layer | Units | Rationale |
|-------|-------|-----------|
| **User-Facing (GUI, API)** | mm, degrees | Intuitive for humans |
| **Internal (core, firmware)** | meters, radians (SI) | Mathematical consistency |
| **Conversion** | `usd.core.units` module | Single source of truth |

---

## Communication Protocol

### Framing: COBS (Consistent Overhead Byte Stuffing)

COBS provides reliable packet framing over byte streams without escape characters:

```
┌─────────────────────────────────────────────────────────────┐
│                      Raw Packet                             │
├──────────┬─────────────────────────────────┬────────────────┤
│  Header  │            Payload              │    CRC-16      │
│ (4 bytes)│         (variable)              │   (2 bytes)    │
└──────────┴─────────────────────────────────┴────────────────┘
                           │
                           v  COBS Encode
┌─────────────────────────────────────────────────────────────┐
│ 0x00 │ COBS-encoded data (no 0x00 inside) │ 0x00            │
└─────────────────────────────────────────────────────────────┘
```

### Packet Header Format

| Byte | Field | Description |
|------|-------|-------------|
| 0 | `msg_type` | Command/response type (see table) |
| 1 | `node_id` | Target node (0 = broadcast, 1-254 = specific) |
| 2-3 | `seq_num` | Sequence number (little-endian) |

### Message Types

| Type | ID | Direction | Description |
|------|----|-----------|-------------|
| `CMD_MOVE_ABS` | 0x10 | Host → MCU | Move to absolute position |
| `CMD_MOVE_REL` | 0x11 | Host → MCU | Move relative distance |
| `CMD_VELOCITY` | 0x12 | Host → MCU | Set target velocity |
| `CMD_STOP` | 0x13 | Host → MCU | Immediate stop |
| `CMD_HOME` | 0x14 | Host → MCU | Start homing sequence |
| `CMD_SET_PID` | 0x20 | Host → MCU | Set PID parameters |
| `CMD_SET_LIMITS` | 0x21 | Host → MCU | Set position/velocity limits |
| `CMD_GET_STATUS` | 0x30 | Host → MCU | Request status packet |
| `CMD_GET_POSITION` | 0x31 | Host → MCU | Request current position |
| `RESP_ACK` | 0x80 | MCU → Host | Command acknowledged |
| `RESP_NAK` | 0x81 | MCU → Host | Command rejected (with error code) |
| `RESP_STATUS` | 0x82 | MCU → Host | Status response |
| `RESP_POSITION` | 0x83 | MCU → Host | Position response |
| `EVT_FAULT` | 0xF0 | MCU → Host | Fault event (async) |
| `EVT_LIMIT` | 0xF1 | MCU → Host | Limit switch triggered (async) |

### CRC-16-CCITT

- **Polynomial:** 0x1021
- **Initial:** 0xFFFF
- **Covers:** Header + Payload (before COBS encoding)

---

## Multi-MCU Synchronization

### v1.0: GPIO Sync Line

```
    ┌─────────────────────────────────────────────┐
    │                 Sync Bus                    │
    │  ┌───────────────────────────────────────┐  │
    │  │            GPIO Line (3.3V)           │  │
    │  └───────────────────────────────────────┘  │
    │       │           │           │             │
    │       v           v           v             │
    │  ┌────────┐  ┌────────┐  ┌────────┐        │
    │  │ MCU 1  │  │ MCU 2  │  │ MCU 3  │        │
    │  │(Master)│  │(Slave) │  │(Slave) │        │
    │  └────────┘  └────────┘  └────────┘        │
    │                                             │
    │  Sync sequence:                             │
    │  1. Master MCU: Pull GPIO HIGH              │
    │  2. All MCUs: Interrupt fires               │
    │  3. All MCUs: Execute motion on same tick   │
    │  4. Master MCU: Release GPIO (or next sync) │
    │                                             │
    │  Latency: ~1-5 µs                           │
    └─────────────────────────────────────────────┘
```

**Host Sync Options:**
- **Raspberry Pi:** Native GPIO (fast, direct)
- **PC:** USB-to-GPIO adapter OR dedicated master MCU

### v1.5+: RS-485 or CAN FD Broadcast

```
    ┌─────────────────────────────────────────────────────┐
    │                  Daisy-Chain Bus                    │
    │  ┌───────────────────────────────────────────────┐  │
    │  │        RS-485 (2-wire) or CAN FD              │  │
    │  └───────────────────────────────────────────────┘  │
    │       │           │           │           │         │
    │       v           v           v           v         │
    │  ┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐    │
    │  │  Host  │  │ MCU 1  │  │ MCU 2  │  │ MCU 3  │    │
    │  │Adapter │  │Node 01 │  │Node 02 │  │Node 03 │    │
    │  └────────┘  └────────┘  └────────┘  └────────┘    │
    │                                                     │
    │  Power Bus (parallel):                              │
    │  - 24V for motors                                   │
    │  - 5V/3.3V for logic                                │
    └─────────────────────────────────────────────────────┘
```

### CAN FD vs Classic CAN

| Feature | Classic CAN | CAN FD |
|---------|-------------|--------|
| **Max Data** | 8 bytes | 64 bytes |
| **Bit Rate** | 1 Mbps | 8 Mbps (data phase) |
| **Latency** | Higher (more frames) | Lower (larger payloads) |
| **ESP32-S3 Support** | Native (TWAI) | Native (TWAI FD) |
| **Recommendation** | Simpler, widely compatible | Preferred for USD |

---

## Firmware Abstraction Interfaces

### Communication Interface

```cpp
class IHostComm {
public:
    virtual ~IHostComm() = default;
    virtual void init() = 0;
    virtual void send(const Packet& pkt) = 0;
    virtual bool receive(Packet& pkt, uint32_t timeout_ms) = 0;
    virtual bool isConnected() const = 0;
};

// Implementations
class UsbSerialComm : public IHostComm { /* v1.0 */ };
class Rs485Comm : public IHostComm { /* v1.5+ */ };
class CanFdComm : public IHostComm { /* v1.5+ */ };
```

### Synchronization Interface

```cpp
class INodeSync {
public:
    virtual ~INodeSync() = default;
    virtual void init() = 0;
    virtual void triggerSync() = 0;    // Master only
    virtual void waitForSync() = 0;    // All nodes
    virtual void registerCallback(void (*cb)()) = 0;
};

// Implementations
class GpioSync : public INodeSync { /* v1.0 */ };
class Rs485Sync : public INodeSync { /* v1.5+ */ };
class CanFdSync : public INodeSync { /* v1.5+ */ };
```

### Driver Interface

```cpp
class IDriver {
public:
    virtual ~IDriver() = default;
    virtual void init() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void step() = 0;
    virtual void setDirection(bool forward) = 0;
    virtual void setMicrosteps(uint8_t ms) = 0;
    virtual void setCurrent(uint16_t mA) = 0;  // If supported
    virtual DriverStatus getStatus() const = 0;
};

// Implementations
class TMC2208Driver : public IDriver { /* Must */ };
class TMC2209Driver : public IDriver { /* Should */ };
class GenericStepDirDriver : public IDriver { /* Fallback */ };
```

### Sensor Interface

```cpp
class ISensor {
public:
    virtual ~ISensor() = default;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual bool isReady() const = 0;
};

class IEncoder : public ISensor {
public:
    virtual int32_t getPosition() const = 0;
    virtual float getVelocity() const = 0;
};

class ILimitSwitch : public ISensor {
public:
    virtual bool isTriggered() const = 0;
    virtual LimitType getType() const = 0;  // MIN, MAX, HOME
};
```

---

## Data Flow

### Command Flow (Host → MCU)

```
┌──────────────┐
│     GUI      │  User clicks "Move to 100mm"
└──────┬───────┘
       │ moveToPosition(100.0, "mm")
       v
┌──────────────┐
│  Controller  │  Convert units, validate
└──────┬───────┘
       │ 0.1 meters (SI)
       v
┌──────────────┐
│   Protocol   │  Build CMD_MOVE_ABS packet
└──────┬───────┘
       │ Header + Payload
       v
┌──────────────┐
│  COBS + CRC  │  Frame and checksum
└──────┬───────┘
       │ 0x00 | encoded | 0x00
       v
┌──────────────┐
│    Serial    │  USB-CDC write
└──────┬───────┘
       │
═══════╪═══════════════════════════════════════════
       │
       v
┌──────────────┐
│  CommTask    │  Receive, COBS decode, CRC check
└──────┬───────┘
       │
       v
┌──────────────┐
│  Protocol    │  Parse command, validate
└──────┬───────┘
       │ Set target position
       v
┌──────────────┐
│ MotionTask   │  Execute trajectory
└──────────────┘
```

### Telemetry Flow (MCU → Host)

```
┌──────────────┐
│ SensorTask   │  Read encoder @ 2.5kHz
└──────┬───────┘
       │ Update position register
       v
┌──────────────┐
│ MotionTask   │  Read position, compute velocity
└──────┬───────┘
       │ Update state machine
       v
┌──────────────┐
│  CommTask    │  On CMD_GET_STATUS or periodic
└──────┬───────┘
       │ Build RESP_STATUS packet
       v
═══════╪═══════════════════════════════════════════
       │
       v
┌──────────────┐
│   Protocol   │  COBS decode, parse response
└──────┬───────┘
       │
       v
┌──────────────┐
│  Controller  │  Convert SI → user units
└──────┬───────┘
       │
       v
┌──────────────┐
│     GUI      │  Update position display
└──────────────┘
```

---

## Technology Stack

| Layer | Technology | Rationale |
|-------|------------|-----------|
| **MCU** | ESP32-S3 | Dual-core, MCPWM, USB-OTG, FreeRTOS, affordable |
| **Firmware Build** | PlatformIO | Cross-platform, library management, testing |
| **RTOS** | FreeRTOS | ESP-IDF native, proven, lightweight |
| **Host Language** | Python 3.10+ | Rapid development, scientific libraries |
| **GUI Framework** | PyQt6 | Feature-rich, cross-platform, Designer tool |
| **Serial Library** | pyserial | Standard, reliable |
| **Math Library** | NumPy | Fast trajectory calculations |
| **Driver IC** | TMC2208 (primary) | Silent, UART config, affordable |
| **Encoder** | AS5600 | Magnetic, contactless, I2C, affordable |
| **Protocol** | COBS + CRC-16 | Robust framing, error detection |

---

## Folder Structure

```
USD/
├── README.md                      # Project overview
├── LICENSE
├── .gitignore
│
├── .github/                       # AI & GitHub configuration
│   ├── copilot-instructions.md
│   ├── agents/
│   ├── instructions/
│   └── processes/
│
├── docs/                          # Documentation
│   ├── README.md
│   ├── specs/                     # SDD specifications (this file)
│   │   ├── DISCOVERY.md
│   │   ├── REQUIREMENTS.md
│   │   ├── ARCHITECTURE.md
│   │   └── TASKS.md
│   ├── dev/                       # Development tracking
│   │   ├── ROADMAP.md
│   │   ├── CHANGELOG.md
│   │   └── KNOWN-ISSUES.md
│   └── guides/                    # User guides
│       ├── getting-started.md
│       └── wiring-guide.md
│
├── firmware/                      # ESP32-S3 firmware
│   ├── platformio.ini
│   ├── src/
│   │   └── main.cpp
│   ├── lib/
│   │   ├── usd_core/
│   │   ├── usd_drivers/
│   │   ├── usd_sensors/
│   │   ├── usd_comms/
│   │   └── usd_safety/
│   └── test/
│
├── software/                      # Python host application
│   ├── pyproject.toml
│   ├── usd/
│   │   ├── core/
│   │   ├── comms/
│   │   ├── drivers/
│   │   └── utils/
│   ├── gui/
│   │   ├── windows/
│   │   ├── widgets/
│   │   ├── controllers/
│   │   └── resources/
│   └── tests/
│
├── common/                        # Shared code (protocol defs)
│   └── protocol/
│       ├── messages.h
│       └── messages.py
│
├── demos/                         # Example projects
│   └── single_axis/
│
└── scripts/                       # Build & utility scripts
    ├── flash.py
    └── test_connection.py
```

---

## Error Handling Strategy

| Error Type | Detection | Response | Recovery |
|------------|-----------|----------|----------|
| **CRC Mismatch** | CommTask | Send NAK with ERR_CRC | Request retransmit |
| **Limit Triggered** | SafetyTask | Stop motion, send EVT_LIMIT | Require explicit clear |
| **Watchdog Timeout** | SafetyTask | Disable motor outputs | Require re-init |
| **Driver Fault** | MotionTask | Stop motion, send EVT_FAULT | Diagnose via status |
| **Comm Timeout** | Host | Retry with exponential backoff | Report to user |
| **Invalid Command** | CommTask | Send NAK with ERR_INVALID | Log for debugging |

### Safe Position Behavior

On critical fault (limit hit, watchdog, driver fault):

1. **Immediate:** Disable step generation (controlled stop preferred)
2. **If encoder present:** Record last known position
3. **State:** Enter FAULT state, reject motion commands
4. **Host notification:** Send EVT_FAULT with fault code and position
5. **Recovery:** Require explicit fault clear + optional homing

---

## Design Decisions

| Decision | Options Considered | Choice | Rationale |
|----------|-------------------|--------|-----------|
| **Control Loop Location** | MCU only, Host+MCU split | MCU only | Real-time guarantees require deterministic execution |
| **Communication Protocol** | JSON, Protobuf, Binary+COBS | Binary+COBS | Efficiency, low overhead, proven reliability |
| **GUI Framework** | Tkinter, PyQt, Web | PyQt6 | Feature-rich, Designer tool, native look |
| **Step Generation** | Timer ISR, MCPWM, RMT | MCPWM primary | Hardware timing, frees CPU |
| **Multi-MCU Sync (v1.0)** | Software timing, GPIO | GPIO line | Sub-10µs latency, simple hardware |
| **Multi-MCU Sync (v1.5+)** | Ethernet, CAN, RS-485 | CAN FD or RS-485 | 64-byte payloads, broadcast support |
| **Encoder Type** | Optical, Mechanical, Magnetic | AS5600 (magnetic) | Contactless, affordable, I2C interface |
| **Driver IC** | A4988, DRV8825, TMC2208 | TMC2208 | Silent operation, UART configuration |

---

## Related Documents

- [DISCOVERY.md](DISCOVERY.md) — Vision and goals
- [REQUIREMENTS.md](REQUIREMENTS.md) — Functional and non-functional requirements
- [TASKS.md](TASKS.md) — Implementation plan
- [SDD Process](../../.github/processes/spec-driven-development.md) — Workflow reference
