# Common: Shared Definitions

> **Purpose:** Single source of truth for protocols, schemas, and constants shared between firmware (C++) and software (Python).

---

## Structure

```
common/
├── README.md                   # This file
└── protocol/                   # Protocol implementations (C source)
    ├── README.md              # Protocol documentation
    ├── cobs.h / cobs.c        # COBS byte stuffing
    ├── crc16.h / crc16.c      # CRC-16-CCITT checksums
    └── messages.h             # Protocol message definitions
```

> **Note:** Protocol files are mirrored to `firmware/lib/usd_protocol/` for PlatformIO builds.
> Python implementations live in `software/usd/comm/`.

---

## Communication Protocol

### Physical Layer

| Parameter | Value |
|-----------|-------|
| **Interface** | USB-CDC (virtual COM port) |
| **Baud Rate** | N/A (USB manages rate) |
| **Transport** | Reliable, bidirectional |

### Framing: COBS (Consistent Overhead Byte Stuffing)

COBS guarantees no zero bytes in the payload, using `0x00` as packet delimiter:

```
┌─────────────────────────────────────────────────────────┐
│ [COBS-encoded payload] [0x00 delimiter]                 │
└─────────────────────────────────────────────────────────┘
```

**Properties:**
- Overhead: 1–2 bytes per 254 payload bytes
- No escape sequences needed
- Self-synchronizing on packet boundaries
- Max packet size: 254 bytes (v1.0)

### Packet Structure

```
┌─────────┬────────┬─────────────┬──────────┐
│ Msg ID  │ Length │   Payload   │  CRC-16  │
│ (1 byte)│(1 byte)│ (0–250 B)   │ (2 bytes)│
└─────────┴────────┴─────────────┴──────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Msg ID | 1 byte | Command/response identifier |
| Length | 1 byte | Payload length (0–250) |
| Payload | 0–250 bytes | Command data (little-endian) |
| CRC-16 | 2 bytes | CRC-16-CCITT (polynomial 0x1021, init 0xFFFF) |

### Message ID Ranges

| Range | Purpose |
|-------|---------|
| 0x00–0x0F | System commands (ping, version, reset) |
| 0x10–0x2F | Motion commands (move, stop, home) |
| 0x30–0x3F | Configuration commands (set/get params) |
| 0x40–0x4F | Telemetry requests |
| 0x80–0xFF | Responses (0x80 + request ID) |

### Data Types (Little-Endian)

| Type | Size | Description |
|------|------|-------------|
| `u8` | 1 byte | Unsigned 8-bit |
| `i16` | 2 bytes | Signed 16-bit |
| `u16` | 2 bytes | Unsigned 16-bit |
| `i32` | 4 bytes | Signed 32-bit |
| `f32` | 4 bytes | IEEE 754 float |

---

## Design Philosophy

Both firmware and software need to agree on:

1. **Message formats** — What bytes mean what
2. **Command IDs** — What commands exist and their parameters
3. **Configuration schemas** — How to describe axes, robots, limits

This folder defines these once, and code generators (or manual sync) ensure both sides match.

---

## Related

- [ARCHITECTURE.md](../docs/specs/ARCHITECTURE.md) — Full system design
- [firmware/lib/usd_protocol/](../firmware/lib/usd_protocol/) — Firmware protocol library (PlatformIO)
- [software/usd/comm/](../software/usd/comm/) — Python protocol implementation
- [software/usd/comm/](../software/usd/) — Python protocol implementation
