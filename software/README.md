# USD Software

> **Platform:** Python 3.10+ | **Host:** PC or Raspberry Pi 5

This folder contains the host software for the Ultimate Stepper Driver system.

---

## Structure

```
software/
├── README.md                   # This file
├── pyproject.toml              # Python project configuration
├── usd/                        # Core library (pip-installable)
│   ├── __init__.py
│   ├── core/                   # Motion planning, kinematics, profiles
│   ├── comm/                   # MCU communication (serial, USB)
│   ├── devices/                # High-level device wrappers (axes, robots)
│   └── utils/                  # Helpers, logging, config
├── gui/                        # Control GUI application
│   └── __init__.py
└── tests/                      # pytest test suite
```

---

## Installation

```bash
# Development install (editable)
cd software
pip install -r requirements.txt
pip install -e .

# Run tests
pytest tests/

# Run GUI (when ready)
python -m gui
```

### Dependencies

See `requirements.txt` for development dependencies:
- `pytest` — Test framework
- `pytest-cov` — Coverage reporting

---

## Modules

| Module | Purpose | Status |
|--------|---------|--------|
| `usd.core` | Motion planning, trajectory generation, kinematics | 📋 Planned |
| `usd.comm` | Serial/USB communication with MCU | 📋 Planned |
| `usd.devices` | High-level axis and robot abstractions | 📋 Planned |
| `usd.utils` | Configuration, logging, helpers | 📋 Planned |
| `gui` | Tkinter/PyQt control interface | 📋 Planned |

---

## Related

- [DISCOVERY.md](../docs/specs/DISCOVERY.md) — Project vision
- [ARCHITECTURE.md](../docs/specs/ARCHITECTURE.md) — System design
- [firmware/](../firmware/) — ESP32 firmware
- [common/](../common/) — Shared protocols
