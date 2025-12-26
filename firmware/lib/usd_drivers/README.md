# USD Drivers Library

Motor driver abstractions for the Ultimate Stepper Driver project.

## Overview

This library provides a hardware abstraction layer for stepper motor drivers and step pulse generators.

### Motor Drivers

| Driver | File | Description | Status |
|--------|------|-------------|--------|
| **IDriver** | `idriver.h` | Abstract interface (all drivers inherit) | ✅ P1 |
| **GenericStepDirDriver** | `step_dir_driver.h/.cpp` | GPIO-based step/dir control | ✅ P1 |
| **TMC2208Driver** | `tmc2208_driver.h/.cpp` | TMC2208 with UART config | 📋 P2 |
| **TMC2209Driver** | `tmc2209_driver.h/.cpp` | TMC2209 with StallGuard | 📋 P2 |

### Step Generators

| Generator | File | Description | Status |
|-----------|------|-------------|--------|
| **IStepGenerator** | `istep_generator.h` | Abstract interface for step pulse generation | ✅ P1 |
| **McpwmStepper** | `mcpwm_stepper.h/.cpp` | MCPWM-based (500 kHz max, hardware timing) | ✅ P1 |
| **TimerStepper** | `timer_stepper.h/.cpp` | Timer ISR fallback (50 kHz max, ISR-based) | ✅ P1 |

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

## IStepGenerator Interface

Step generators create precise step pulses at a specified frequency:

```cpp
namespace usd {

class IStepGenerator {
public:
    // Control
    virtual bool init() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual StepGenState getState() const = 0;

    // Frequency
    virtual bool setFrequency(uint32_t frequency_hz) = 0;
    virtual uint32_t getFrequency() const = 0;
    virtual uint32_t getMaxFrequency() const = 0;
    virtual uint32_t getMinFrequency() const = 0;

    // Step Counting
    virtual uint32_t getStepCount() const = 0;
    virtual void resetStepCount() = 0;
    virtual void setTargetSteps(uint32_t target_steps) = 0;

    // Configuration
    virtual int8_t getStepPin() const = 0;
    virtual uint16_t getPulseWidth() const = 0;
};

}  // namespace usd
```

## Step Generator Comparison

| Feature | McpwmStepper | TimerStepper |
|---------|--------------|--------------|
| Max Frequency | 500 kHz | 50 kHz |
| Timing Method | Hardware PWM | Timer ISR |
| CPU Usage | Very Low | Low-Medium |
| Jitter | None | Minimal (~1µs) |
| Availability | ESP32 only | ESP32 (fallback) |

### McpwmStepper Example

```cpp
#include "mcpwm_stepper.h"

usd::McpwmConfig config = {
    .unit = 0,
    .timer = 0,
    .operator_id = 0,
    .output = 0
};

usd::McpwmStepper stepper(18, config);  // GPIO 18
stepper.init();
stepper.setFrequency(100000);  // 100 kHz
stepper.start();
// ... motor moving at high speed ...
stepper.stop();
```

### TimerStepper Example

```cpp
#include "timer_stepper.h"

usd::TimerConfig config = usd::TimerConfig::defaultConfig();
usd::TimerStepper stepper(18, config);  // GPIO 18

stepper.init();
stepper.setFrequency(25000);  // 25 kHz
stepper.start();
// ... motor moving ...
stepper.stop();
```

## Status

| Component | Status |
|-----------|--------|
| IDriver interface | ✅ Complete |
| IStepGenerator interface | ✅ Complete |
| GenericStepDirDriver | 🟡 P1.2 |
| McpwmStepper | ✅ Complete (P1.3) |
| TimerStepper | ✅ Complete (P1.4) |
| TMC2208Driver | 📋 P2.2 |
| TMC2209Driver | 📋 P2.5 |

## Dependencies

- ESP-IDF or Arduino framework (for GPIO)
- FreeRTOS (for delays and task-safe operations)

## Related

- [Architecture](../../../docs/specs/ARCHITECTURE.md)
- [TASKS](../../../docs/specs/TASKS.md)
