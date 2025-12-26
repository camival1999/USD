# USD Drivers Library

Motor driver abstractions for the Ultimate Stepper Driver project.

## Overview

This library provides a hardware abstraction layer for stepper motor drivers:

| Driver | File | Description |
|--------|------|-------------|
| **IDriver** | `idriver.h` | Abstract interface (all drivers inherit) |
| **GenericStepDirDriver** | `step_dir_driver.h/.cpp` | GPIO-based step/dir control |
| **TMC2208Driver** | `tmc2208_driver.h/.cpp` | TMC2208 with UART config |
| **TMC2209Driver** | `tmc2209_driver.h/.cpp` | TMC2209 with StallGuard |

## IDriver Interface

All drivers implement this interface:

```cpp
namespace usd {

class IDriver {
public:
    // Enable/Disable
    virtual bool enable() = 0;
    virtual void disable() = 0;
    virtual bool isEnabled() const = 0;
    virtual DriverState getState() const = 0;

    // Direction
    virtual void setDirection(Direction dir) = 0;
    virtual Direction getDirection() const = 0;

    // Stepping
    virtual bool step() = 0;
    virtual uint32_t stepMultiple(uint32_t count, uint32_t rate) = 0;

    // Position
    virtual int32_t getPosition() const = 0;
    virtual void setPosition(int32_t pos) = 0;
    virtual void resetPosition();

    // Faults
    virtual bool isFault() const = 0;
    virtual bool clearFault() = 0;
    virtual uint8_t getFaultCode() const = 0;
};

}  // namespace usd
```

## Configuration Structs

```cpp
struct DriverPins {
    int8_t step_pin;          // GPIO for step pulse
    int8_t dir_pin;           // GPIO for direction
    int8_t enable_pin;        // GPIO for enable
    bool enable_active_low;   // Typical: true
    bool dir_invert;          // Invert direction logic
};

struct StepTiming {
    uint16_t step_pulse_us;   // Pulse width (typ. 2-5 µs)
    uint16_t dir_setup_us;    // Dir setup before step (typ. 5-20 µs)
    uint16_t enable_delay_ms; // Delay after enable (typ. 5-10 ms)
};
```

## Usage Example

```cpp
#include "step_dir_driver.h"

// Configure pins
usd::DriverPins pins = {
    .step_pin = 18,
    .dir_pin = 19,
    .enable_pin = 21,
    .enable_active_low = true,
    .dir_invert = false
};

// Configure timing
usd::StepTiming timing = {
    .step_pulse_us = 3,
    .dir_setup_us = 10,
    .enable_delay_ms = 5
};

// Create and use driver
usd::GenericStepDirDriver driver(pins, timing);
driver.enable();
driver.setDirection(usd::Direction::CW);
driver.stepMultiple(200, 1000);  // 200 steps at 1kHz
driver.disable();
```

## Status

| Component | Status |
|-----------|--------|
| IDriver interface | ✅ Complete |
| GenericStepDirDriver | 🟡 P1.2 |
| MCPWM integration | 🟡 P1.3 |
| TMC2208Driver | 📋 P2.2 |
| TMC2209Driver | 📋 P2.5 |

## Dependencies

- ESP-IDF or Arduino framework (for GPIO)
- FreeRTOS (for delays and task-safe operations)

## Related

- [Architecture](../../../docs/specs/ARCHITECTURE.md)
- [TASKS](../../../docs/specs/TASKS.md)
