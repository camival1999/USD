# Common: Shared Definitions

> **Purpose:** Single source of truth for protocols, schemas, and constants shared between firmware (C++) and software (Python).

---

## Structure

```
common/
├── README.md                   # This file
├── protocols/                  # Message definitions, packet formats
│   ├── messages.md            # Human-readable protocol spec
│   └── messages.json          # Machine-parseable message definitions
└── schemas/                    # Configuration schemas
    └── config.schema.json     # Axis/robot configuration schema
```

---

## Design Philosophy

Both firmware and software need to agree on:

1. **Message formats** — What bytes mean what
2. **Command IDs** — What commands exist and their parameters
3. **Configuration schemas** — How to describe axes, robots, limits

This folder defines these once, and code generators (or manual sync) ensure both sides match.

---

## Protocol Design (TBD)

| Aspect | Options to Consider |
|--------|---------------------|
| **Encoding** | Binary (compact, fast) vs JSON (readable, flexible) vs Protobuf (structured, versioned) |
| **Framing** | SLIP, COBS, length-prefixed, or custom delimiters |
| **Reliability** | Checksums, ACK/NAK, sequence numbers |
| **Timing** | Sync vs async, streaming vs request-response |

> Decision to be made in ARCHITECTURE phase.

---

## Related

- [DISCOVERY.md](../docs/specs/DISCOVERY.md) — Project vision
- [ARCHITECTURE.md](../docs/specs/ARCHITECTURE.md) — System design (protocol decisions here)
- [firmware/](../firmware/) — C++ consumer
- [software/](../software/) — Python consumer
