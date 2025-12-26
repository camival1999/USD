# USD Protocol Library

> **Type:** PlatformIO Library | **Version:** 0.1.0

This library contains the core protocol layer for USD firmware.

## Components

| File | Description |
|------|-------------|
| `cobs.h/.c` | COBS (Consistent Overhead Byte Stuffing) encoding |
| `crc16.h/.c` | CRC-16-CCITT checksum calculation |
| `messages.h` | Protocol message definitions and structures |

## Source

These files are **duplicates** of `common/protocol/` for PlatformIO compatibility.
The canonical versions are in `common/protocol/` and shared with Python.

When modifying protocol code:
1. Edit files in `common/protocol/`
2. Copy updated files here
3. Run both Python and C tests

## Testing

```bash
# Native tests (fast, on host machine)
pio test -e native

# On-target tests (requires ESP32 connected)
pio test -e esp32s3
```

## Related

- [common/protocol/README.md](../../../common/protocol/README.md) - Protocol specification
- [ARCHITECTURE.md](../../../docs/specs/ARCHITECTURE.md) - System design
