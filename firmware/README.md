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

# Run unit tests
pio test

# Monitor serial output
pio device monitor
```

---

## Libraries

| Library | Purpose | Status |
|---------|---------|--------|
| `usd_core` | Control loops (PID, feedforward), motion profiles | 📋 Planned |
| `usd_drivers` | Hardware driver abstractions | 📋 Planned |
| `usd_sensors` | Sensor reading and processing | 📋 Planned |
| `usd_comms` | Host communication protocol | 📋 Planned |
| `usd_safety` | Watchdog, thermal monitoring, alerts | 📋 Planned |

---

## Related

- [DISCOVERY.md](../docs/specs/DISCOVERY.md) — Project vision
- [ARCHITECTURE.md](../docs/specs/ARCHITECTURE.md) — System design
- [software/](../software/) — Python host software
