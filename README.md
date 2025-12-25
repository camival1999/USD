# USD - Ultimate Stepper Driver

> A full-stack, DIY-friendly motion control system for precision multi-axis stepper motor coordination.

[![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)]()
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-green.svg)]()
[![SDD](https://img.shields.io/badge/developed_with-SDD_+_AI-purple.svg)]()

> **Note:** This project is being developed using the **Spec-Driven Development (SDD)** framework with AI assistance (GitHub Copilot). All specifications, architecture decisions, and implementation plans are documented in [`docs/specs/`](docs/specs/) for full transparency.

---

## 🎯 Vision

USD bridges hobbyist hardware (ESP32 + TMC drivers) with high-level software (Python/PC/RPi), enabling precise multi-axis coordination for personal robotics projects—without industrial budgets.

**Key Features:**

- 🔧 **Universal Driver Support** — TMC2209, TMC2208, and extensible to other drivers
- 📐 **Precision Control** — Nested loops: torque → velocity → position
- 🤖 **Multi-Axis Coordination** — Up to 12 synchronized axes
- 🛡️ **Safety Built-In** — Thermal monitoring, overcurrent protection, watchdog
- 💻 **Full Stack** — MCU firmware (C++) + host software (Python) + GUI

---

## 📁 Project Structure

```
USD/
├── firmware/           # ESP32-S3 firmware (PlatformIO/C++)
│   ├── src/           # Main application
│   └── lib/           # Core libraries (drivers, sensors, control)
├── software/          # Python host software
│   ├── usd/           # Core library (pip-installable)
│   └── gui/           # Control interface
├── common/            # Shared protocols & schemas
├── demos/             # Example applications
├── docs/              # Documentation & specs
│   ├── specs/         # SDD specifications
│   └── dev/           # Development tracking
└── scripts/           # Build & utility scripts
```

---

## 🚀 Quick Start

### Prerequisites

- [PlatformIO](https://platformio.org/) for firmware development
- Python 3.10+ for host software
- ESP32-S3 development board
- TMC2209 or TMC2208 stepper driver

### Firmware

```bash
cd firmware
pio run --target upload
pio device monitor
```

### Software

```bash
cd software
pip install -e .
python -m gui
```

---

## 📊 Performance Targets

| Metric | Target |
|--------|--------|
| Max step rate | 100 kHz |
| Position accuracy | ±0.05° |
| Control loop (position) | 100 Hz – 5 kHz |
| Max axes | 12 |

---

## 🗺️ Development Status

| Phase | Status | Description |
|-------|--------|-------------|
| **Discovery** | ✅ Complete | Vision, scope, constraints, distributed architecture |
| **Requirements** | ✅ Complete | 70 requirements (37 Must, 26 Should, 7 Could) |
| **Architecture** | ✅ Complete | Firmware + Software design, COBS protocol, multi-MCU sync |
| **Tasks** | ✅ Complete | 10 phases, ~238 hours, 6 milestones defined |
| **Build** | 🟡 Starting | Implementation begins with Phase P0 |

See [docs/specs/](docs/specs/) for detailed specifications and [docs/dev/ROADMAP.md](docs/dev/ROADMAP.md) for milestones.

---

## 📖 Documentation

| Document | Purpose |
|----------|---------|
| [DISCOVERY.md](docs/specs/DISCOVERY.md) | Project vision and scope |
| [REQUIREMENTS.md](docs/specs/REQUIREMENTS.md) | Feature requirements |
| [ARCHITECTURE.md](docs/specs/ARCHITECTURE.md) | System design |
| [ROADMAP.md](docs/dev/ROADMAP.md) | Development timeline |
| [CHANGELOG.md](docs/dev/CHANGELOG.md) | Version history |

---

## 🤝 Contributing

This is a personal hobby project, but suggestions and ideas are welcome!

---

## 📝 License

**Proprietary** — All rights reserved. See [LICENSE](LICENSE) for details.

You may view the source code for educational purposes, but copying, modifying, or distributing the code without permission is prohibited.

For licensing inquiries: camival1999@gmail.com

---

*Built with ❤️ and too many stepper motors.*
