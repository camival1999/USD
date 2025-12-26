# USD Firmware

> **Platform:** ESP32-S3 via PlatformIO | **Language:** C++

This folder contains the MCU firmware for the Ultimate Stepper Driver system.

---

## Structure

```
firmware/
├── platformio.ini              # PlatformIO project configuration
├── README.md                   # This file
├── src/                        # Main application source
│   └── main.cpp               # Entry point
├── include/                    # Project headers
├── lib/                        # Private libraries
│   ├── usd_core/              # Core control algorithms (PID, motion profiles)
│   ├── usd_drivers/           # Driver abstractions (TMC2209, TMC2208, etc.)
│   ├── usd_sensors/           # Sensor abstractions (AS5600, IMU, etc.)
│   ├── usd_comms/             # Communication protocols (UART, USB, etc.)
│   └── usd_safety/            # Watchdog, monitoring, alerts
└── test/                       # PlatformIO unit tests
```

---

## Build

```bash
# Build for ESP32-S3
pio run

# Upload to connected board
pio run --target upload

# Run unit tests (native, requires GCC/MinGW on Windows)
pio test -e native

# Monitor serial output
pio device monitor
```

### Native Test Requirements (Windows)

Native tests run on your development machine for fast protocol validation.
They require a C/C++ compiler:

1. **Install MinGW-w64** (recommended):
   - Download from [winlibs.com](https://winlibs.com/) or use `winget install mingw`
   - Add to PATH: `C:\mingw64\bin`

2. **Or use Visual Studio Build Tools**:
   - Already installed at `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`
   - Requires additional PlatformIO configuration

---

## Libraries

| Library | Purpose | Status |
|---------|---------|--------|
| `usd_core` | Motion controller, trajectory profiles (trapezoidal, S-curve) | ✅ Complete (P1) |
| `usd_drivers` | IDriver interface, GenericStepDirDriver, MCPWM stepper | ✅ Complete (P1) |
| `usd_protocol` | COBS, CRC-16, message definitions | ✅ Complete (P0) |
| `usd_sensors` | Sensor reading and processing | 📋 Planned (P3) |
| `usd_comms` | Host communication protocol | 📋 Planned (P5) |
| `usd_safety` | Watchdog, thermal monitoring, alerts | 📋 Planned (P8) |

---

## Related

- [DISCOVERY.md](../docs/specs/DISCOVERY.md) — Project vision
- [ARCHITECTURE.md](../docs/specs/ARCHITECTURE.md) — System design
- [software/](../software/) — Python host software
