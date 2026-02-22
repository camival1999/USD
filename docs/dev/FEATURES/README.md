# Features

This folder contains per-feature tracking documents maintained by the **Scribe** agent.

## How to Use

Each feature gets its own markdown file tracking:
- Current status
- Implementation progress
- Related files and issues
- Key decisions

## Feature Template

Create new feature files using this template:

```markdown
# Feature: [Feature Name]

| Field | Value |
|-------|-------|
| **Status** | 📋 Planned / 🟡 In Progress / ✅ Complete / ❌ Cancelled |
| **Priority** | High / Medium / Low |
| **Started** | YYYY-MM-DD or — |
| **Completed** | YYYY-MM-DD or — |
| **Version** | Target version (e.g., v0.2.0) |
| **Related Issues** | BUG-XXX, BUG-XXX |

## Description

[What this feature does and why it's needed]

## Requirements

[Link to or list relevant requirements from REQUIREMENTS.md]
- FR-XXX: [Description]
- FR-XXX: [Description]

## Implementation Notes

[Key decisions, gotchas, dependencies, technical approach]

## Files Changed

[List of files created or modified for this feature]
- `src/component/file.py` — [What was done]
- `tests/unit/test_file.py` — [Tests added]

## Testing

[How to verify this feature works]
- [ ] Unit tests passing
- [ ] Integration tests passing
- [ ] Manual verification: [steps]

## Notes

[Any additional context, future improvements, related work]
```

## Status Icons

| Icon | Status |
|------|--------|
| 📋 | Planned — Not yet started |
| 🟡 | In Progress — Actively being worked on |
| ✅ | Complete — Implemented and verified |
| ❌ | Cancelled — Will not be implemented |
| 🚫 | Blocked — Waiting on dependency |

## Current Features

| Feature | Status | Priority |
|---------|--------|----------|
| [P0-foundation.md](P0-foundation.md) | ✅ Complete | High |
| [P1-core-motion.md](P1-core-motion.md) | ✅ Complete | High |
| [P2-driver-integration.md](P2-driver-integration.md) | ⏸️ Standby | High |

---

## Related

- [ROADMAP.md](../ROADMAP.md) — Version planning
- [TASKS.md](../../specs/TASKS.md) — Implementation tasks
- [REQUIREMENTS.md](../../specs/REQUIREMENTS.md) — Feature requirements
