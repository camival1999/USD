# USD Communications Library

Host-MCU communication protocols.

## Protocol Overview

| Aspect | Implementation |
|--------|----------------|
| **Interface** | USB-CDC (virtual COM port) |
| **Framing** | COBS (Consistent Overhead Byte Stuffing) |
| **Checksum** | CRC-16-CCITT (polynomial 0x1021, init 0xFFFF) |
| **Delimiter** | 0x00 (zero byte) |
| **Max Packet** | 254 bytes |

## Packet Structure

```
┌─────────┬────────┬─────────────┬──────────┐
│ Msg ID  │ Length │   Payload   │  CRC-16  │
│ (1 byte)│(1 byte)│ (0–250 B)   │ (2 bytes)│
└─────────┴────────┴─────────────┴──────────┘
```

## Components

| File | Purpose | Status |
|------|---------|--------|
| `cobs.h/cpp` | COBS encode/decode | 📋 Planned |
| `crc16.h/cpp` | CRC-16-CCITT calculation | 📋 Planned |
| `protocol.h/cpp` | Packet assembly/parsing | 📋 Planned |
| `usb_cdc.h/cpp` | USB communication handler | 📋 Planned |
| `command_handler.h/cpp` | Command dispatch | 📋 Planned |

## Features

- Serial/USB communication with host
- COBS message framing and parsing
- CRC validation and error detection
- Command handling and dispatch
- Telemetry streaming

## Status

📋 Planned - Implementation begins in Phase P1

## Related

- [common/README.md](../../../common/README.md) — Full protocol specification
- [ARCHITECTURE.md](../../../docs/specs/ARCHITECTURE.md) — System design
