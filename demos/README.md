# USD Demos

> **Purpose:** Example applications demonstrating USD capabilities.

---

## Available Demos

| Demo | Description | Status |
|------|-------------|--------|
| `single_axis/` | Basic single motor positioning | 📋 Planned |
| `multi_axis/` | Coordinated multi-axis motion | 📋 Planned |
| `pick_and_place/` | Complete pick-and-place robot demo | 📋 Planned |

---

## Running Demos

```bash
# Single axis demo
cd demos/single_axis
python run.py

# Multi-axis demo  
cd demos/multi_axis
python run.py

# Pick and place demo
cd demos/pick_and_place
python run.py
```

---

## Hardware Requirements

Each demo will document its specific hardware requirements, including:

- Number and type of motors/drivers
- Sensor requirements
- Wiring diagrams
- Safety considerations

---

## Creating New Demos

1. Create a folder: `demos/your_demo_name/`
2. Add a `README.md` with hardware requirements and instructions
3. Add a `run.py` entry point
4. Document any configuration needed

---

## Related

- [DISCOVERY.md](../docs/specs/DISCOVERY.md) — Project vision
- [software/](../software/) — Python library used by demos
- [firmware/](../firmware/) — ESP32 firmware for demos
