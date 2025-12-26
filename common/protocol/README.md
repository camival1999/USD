# USD Protocol - Shared Definitions

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

This folder contains the **single source of truth** for the USD communication protocol.
Both C++ (firmware) and Python (host software) implementations derive from these definitions.

## ⚠️ Important: Source of Truth

This folder (`common/protocol/`) is the **canonical source** for protocol code.

| Location | Purpose | Status |
|----------|---------|--------|
| `common/protocol/` | **Source of truth** | Edit here |
| `firmware/lib/usd_protocol/` | PlatformIO copy | Sync from common/ |
| `software/usd/comm/` | Python implementation | Mirrors common/ |

**When modifying protocol code:**
1. Edit files in `common/protocol/`
2. Copy `.c` and `.h` files to `firmware/lib/usd_protocol/`
3. Update Python mirrors in `software/usd/comm/`
4. Run tests in both environments

## Files

| File | Purpose | Used By |
|------|---------|---------|
| `cobs.h` / `cobs.c` | COBS framing | Firmware |
| `crc16.h` / `crc16.c` | CRC-16-CCITT | Firmware |
| `messages.h` | Message IDs and structures | Firmware |

Python mirrors are in `software/usd/comm/`:
- `cobs.py` — mirrors `cobs.h`
- `crc16.py` — mirrors `crc16.h`
- `messages.py` — mirrors `messages.h`

## Protocol Overview

### Framing

```
┌─────────────────────────────────────────────────────────┐
│ [COBS-encoded packet] [0x00 delimiter]                  │
└─────────────────────────────────────────────────────────┘
```

### Packet Structure (before COBS encoding)

```
┌─────────┬────────┬─────────────┬──────────┐
│ Msg ID  │ Length │   Payload   │  CRC-16  │
│ (1 byte)│(1 byte)│ (0–250 B)   │ (2 bytes)│
└─────────┴────────┴─────────────┴──────────┘
```

### Message ID Ranges

| Range | Purpose |
|-------|---------|
| 0x00–0x0F | System commands (ping, version, reset) |
| 0x10–0x2F | Motion commands (move, stop, home) |
| 0x30–0x3F | Configuration commands |
| 0x40–0x4F | Telemetry requests |
| 0x80–0xFF | Responses (0x80 + request ID) |

## Keeping In Sync

When modifying the protocol:

1. Update the C header file first (`messages.h`)
2. Update the Python mirror (`messages.py`)
3. Ensure enum values and struct layouts match exactly
4. Run protocol tests to verify compatibility

## Testing

```bash
# Python protocol tests
cd software
pytest tests/test_protocol.py

# C protocol tests (native)
cd firmware
pio test -e native
```
