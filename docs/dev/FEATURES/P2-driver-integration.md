# Feature: P2 Driver Integration

| Field | Value |
|-------|-------|
| **Status** | ⏸️ Standby (Awaiting Hardware Confirmation) |
| **Priority** | High |
| **Started** | 2025-01-18 |
| **Completed** | — |
| **Estimated Effort** | 30h (26h native + 4h hardware validation) |
| **New Tests** | ~58 native tests |

---

## Description

Implement TMC2208/TMC2209 UART-configured stepper drivers with:
- 1-wire bidirectional UART protocol
- CRC8 validation
- Microstepping configuration (1-256)
- Current limiting (IRUN/IHOLD)
- StallGuard sensorless stall detection (TMC2209)
- Multi-driver addressing (0-3 via MS1/MS2)
- Driver factory for runtime selection

---

## ⏸️ Standby Checklist

**Before resuming P2 implementation, confirm:**

| # | Item | Status | Your Response |
|---|------|--------|---------------|
| 1 | Register addresses from TMC2208 datasheet v1.10 look correct? | ⏳ Pending | |
| 2 | Which ESP32-S3 GPIO for TMC UART? | ✅ Answered | Configurable, default TX=1, RX=2 |
| 3 | Which GPIOs for MS1/MS2? | ✅ Answered | Hardwired via jumpers (user responsibility) |
| 4 | TMC2208 wiring confirmation? | ⏳ Pending | User to check next week |

---

## Architecture

```
┌────────────────────────────────────────────────────────────────────────┐
│                           MotionController                              │
│                     (uses IDriver* abstraction)                         │
└───────────────────────────────────────────────────────────────────────┘
                                │
        ┌───────────────────────┼───────────────────────┐
        │                       │                       │
        ▼                       ▼                       ▼
┌───────────────┐     ┌─────────────────┐     ┌─────────────────┐
│GenericStepDir │     │  TMC2208Driver  │     │  TMC2209Driver  │
│   Driver      │     │   (IDriver)     │     │   (IDriver)     │
│   (GPIO)      │     │                 │     │ extends 2208    │
└───────────────┘     └────────┬────────┘     └────────┬────────┘
    [P1 ✅]                     │                       │
                               │                       │
                               ▼                       ▼
                        ┌─────────────────────────────────────┐
                        │           TmcUartInterface          │
                        │  • CRC8 calculation                 │
                        │  • Datagram encode/decode           │
                        │  • Half-duplex UART I/O             │
                        │  • Multi-address support (0-3)      │
                        └─────────────────────────────────────┘
                                        │
                        ┌───────────────┴───────────────┐
                        │    ESP32 UART (Half-Duplex)   │
                        │  1-wire with 1kΩ resistor     │
                        │  Configurable TX/RX pins      │
                        │  MS1/MS2 hardwired by user    │
                        └───────────────────────────────┘
```

---

## Implementation Order

### Sprint 1: Foundation (P2.1) - 4h

| Task | Description | Files |
|------|-------------|-------|
| CRC8 | TMC polynomial 0x07 | `tmc_crc8.h/cpp` |
| Datagram | Encode/decode sync+addr+reg+data+crc | `tmc_uart.h/cpp` |
| Half-Duplex | ESP32 UART abstraction (configurable pins) | `tmc_uart.h/cpp` |
| Multi-Address | Support addresses 0-3 | `tmc_uart.h/cpp` |
| Tests | Unit tests for CRC8, datagrams | `test_tmc_uart/` |

**Deliverables:**
- `firmware/lib/usd_drivers/tmc_crc8.h/cpp` (~50 lines)
- `firmware/lib/usd_drivers/tmc_uart.h/cpp` (~250 lines)
- `firmware/test/test_tmc_crc8/test_tmc_crc8.cpp` (~6 tests)
- `firmware/test/test_tmc_uart/test_tmc_uart.cpp` (~6 tests)

---

### Sprint 2: TMC2208 Driver (P2.2-P2.4) - 11h

| Task | Description | Est |
|------|-------------|-----|
| P2.2 | TMC2208Driver implementing IDriver | 6h |
| P2.3 | Microstepping (CHOPCONF.mres) | 2h |
| P2.4 | Current limits (IHOLD_IRUN) | 3h |

**Files:**
- `firmware/lib/usd_drivers/tmc_registers.h` - Register definitions
- `firmware/lib/usd_drivers/tmc2208_driver.h/cpp` (~400 lines)
- `firmware/test/test_tmc2208/test_tmc2208.cpp` (~20 tests)

**Key Registers (TMC2208 Datasheet v1.10):**

| Register | Address | Purpose |
|----------|---------|---------|
| GCONF | 0x00 | General config (I_scale_analog, pdn_disable, mstep_reg_select) |
| GSTAT | 0x01 | Status (reset, drv_err, uv_cp) - read-clear |
| IHOLD_IRUN | 0x10 | Current settings (IHOLD[4:0], IRUN[4:0], IHOLDDELAY[3:0]) |
| CHOPCONF | 0x6C | Chopper config + mres (MRES[3:0] = bits 27:24) |
| DRV_STATUS | 0x6F | Driver status (stst, olb, ola, s2gb, s2ga, otpw, ot) |
| PWMCONF | 0x70 | PWM config for stealthChop |

---

### Sprint 3: TMC2209 + StallGuard (P2.5-P2.6) - 8h

| Task | Description | Est |
|------|-------------|-----|
| P2.5 | TMC2209Driver (extends 2208) | 4h |
| P2.6 | StallGuard (SGTHRS, SG_RESULT) | 4h |

**Files:**
- `firmware/lib/usd_drivers/tmc2209_driver.h/cpp` (~300 lines)
- `firmware/test/test_tmc2209/test_tmc2209.cpp` (~18 tests)

**TMC2209-Specific Registers:**

| Register | Address | Purpose |
|----------|---------|---------|
| SGTHRS | 0x40 | StallGuard threshold (0-255) |
| SG_RESULT | 0x41 | StallGuard result (read-only, 10-bit) |
| TCOOLTHRS | 0x14 | Coolstep threshold velocity |
| COOLCONF | 0x42 | Coolstep config |

---

### Sprint 4: Driver Factory (P2.7) - 3h

| Task | Description |
|------|-------------|
| Factory | Runtime driver selection with address param |
| Config | Struct-based driver config |
| Tests | Factory pattern tests |

**Files:**
- `firmware/lib/usd_drivers/driver_factory.h/cpp` (~150 lines)
- `firmware/test/test_driver_factory/test_driver_factory.cpp` (~8 tests)

---

### Sprint 5: Hardware Validation (NEW) - 4h

**Objective:** Validate UART communication with real TMC2208 hardware

| Task | Description | Est |
|------|-------------|-----|
| Wiring | Connect ESP32-S3 ↔ TMC2208 | 0.5h |
| Basic Comms | Read GCONF, GSTAT registers | 1h |
| Write Test | Set microstepping, verify with read-back | 1h |
| Current Test | Set IRUN/IHOLD, verify motor holds | 1h |
| Integration | Validate step/dir with UART config | 0.5h |

**Hardware Test Checklist:**
- [ ] UART TX→RX echo works (loopback test)
- [ ] Read GCONF returns non-zero value
- [ ] Write CHOPCONF.mres, read back matches
- [ ] Write IHOLD_IRUN, motor holds position
- [ ] Step/dir works with UART configuration active

---

## Multi-Driver Addressing (MS1/MS2)

TMC2209 supports up to 4 drivers on shared UART via MS1/MS2 pins:

| MS2 | MS1 | Address |
|-----|-----|---------|
| LOW | LOW | 0 |
| LOW | HIGH | 1 |
| HIGH | LOW | 2 |
| HIGH | HIGH | 3 |

**Note:** MS1/MS2 are hardwired via jumpers (user responsibility). Software only needs to know the resulting address (0-3).

---

## UART Configuration

**Configurable pins** (any ESP32-S3 GPIO):

```cpp
struct TmcUartConfig {
    int8_t tx_pin = 1;          // Default TX (configurable)
    int8_t rx_pin = 2;          // Default RX (configurable)
    uint8_t address = 0;        // 0-3 (set via MS1/MS2 jumpers)
    uint32_t baud_rate = 115200;
};
```

**1-Wire Mode:** For true 1-wire half-duplex, TX and RX are connected together with a 1kΩ resistor. ESP32 handles this by switching between TX/RX modes.

---

## Testing Strategy

### Native Testing (Mock UART)

```cpp
class ITmcUart {
public:
    virtual bool writeRegister(uint8_t addr, uint8_t reg, uint32_t value) = 0;
    virtual bool readRegister(uint8_t addr, uint8_t reg, uint32_t& value) = 0;
};

class MockTmcUart : public ITmcUart {
    std::map<uint16_t, uint32_t> registers_;  // key = (addr << 8) | reg
public:
    bool writeRegister(uint8_t addr, uint8_t reg, uint32_t value) override;
    bool readRegister(uint8_t addr, uint8_t reg, uint32_t& value) override;
};
```

### Test Coverage

| Suite | Tests | Focus |
|-------|-------|-------|
| test_tmc_crc8 | 6 | CRC8 calculation |
| test_tmc_uart | 6 | Datagram encode/decode, multi-address |
| test_tmc2208 | 20 | Driver interface, registers |
| test_tmc2209 | 18 | StallGuard, mode switching |
| test_driver_factory | 8 | Factory pattern |
| **Native Total** | **58** | |
| **Hardware Validation** | Manual | Real ESP32 + TMC2208 |

---

## File Structure (After P2)

```
firmware/lib/usd_drivers/
├── idriver.h                    [P1 - exists]
├── istep_generator.h            [P1 - exists]
├── step_dir_driver.h/cpp        [P1 - exists]
├── mcpwm_stepper.h/cpp          [P1 - exists]
├── timer_stepper.h/cpp          [P1 - exists]
├── tmc_crc8.h/cpp               [P2.1 - NEW]
├── tmc_uart.h/cpp               [P2.1 - NEW]
├── tmc_registers.h              [P2.2 - NEW]
├── tmc2208_driver.h/cpp         [P2.2 - NEW]
├── tmc2209_driver.h/cpp         [P2.5 - NEW]
├── driver_factory.h/cpp         [P2.7 - NEW]
└── README.md                    [update]

firmware/test/
├── test_tmc_crc8/               [P2.1 - NEW, 6 tests]
├── test_tmc_uart/               [P2.1 - NEW, 6 tests]
├── test_tmc2208/                [P2.2 - NEW, 20 tests]
├── test_tmc2209/                [P2.5 - NEW, 18 tests]
└── test_driver_factory/         [P2.7 - NEW, 8 tests]
```

---

## Definition of Done

- [ ] All 58 new native tests passing
- [ ] Zero Pylance/linter warnings (Code Quality rule)
- [ ] CRC8 verified against TMC datasheet examples
- [ ] Mock UART enables full native testing
- [ ] Configurable UART pins (default TX=1, RX=2)
- [ ] Multi-address support (0-3) implemented
- [ ] Driver factory correctly instantiates all driver types
- [ ] **Hardware validation passed** (ESP32 + TMC2208)
- [ ] README.md updated with new driver classes
- [ ] CHANGELOG.md entry for P2
- [ ] ROADMAP.md P2 marked complete

---

## ITmcDriver Interface Extension

```cpp
class ITmcDriver : public IDriver {
public:
    // Microstepping (P2.3)
    virtual bool setMicrostepping(uint16_t microsteps) = 0;  // 1,2,4,8,16,32,64,128,256
    virtual uint16_t getMicrostepping() const = 0;
    
    // Current limiting (P2.4)
    virtual bool setRunCurrent(uint8_t percent) = 0;   // 0-100%
    virtual bool setHoldCurrent(uint8_t percent) = 0;  // 0-100%
    virtual uint8_t getRunCurrent() const = 0;
    virtual uint8_t getHoldCurrent() const = 0;
    
    // Diagnostics (FR-304)
    virtual uint32_t readStatus() = 0;
    virtual bool isStalled() const = 0;  // TMC2209 only
};
```

---

## Resume Instructions

**When you return, say:** "Resume P2" with answers to:
1. TMC2208 wiring confirmation (which pins connected where)
2. Any adjustments to register addresses

The plan will continue from Sprint 1: P2.1 TMC UART Interface.

---

## Related

- [P0-foundation.md](P0-foundation.md) - Foundation phase (complete)
- [P1-core-motion.md](P1-core-motion.md) - Core motion phase (complete)
- [TASKS.md](../../specs/TASKS.md) - Full task breakdown
- [REQUIREMENTS.md](../../specs/REQUIREMENTS.md) - Requirements traceability
