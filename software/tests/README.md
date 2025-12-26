# USD Software Tests

Test suite for the USD Python package.

## Running Tests

```bash
# From the software/ directory
cd software

# Install in development mode
pip install -e ".[dev]"

# Run all tests
pytest

# Run with coverage
pytest --cov=usd --cov-report=html

# Run specific test file
pytest tests/test_package.py

# Run with verbose output
pytest -v
```

## Test Structure

```
tests/
├── conftest.py          # Shared fixtures
├── test_package.py      # Import/structure tests
├── test_comm/           # Communication module tests (P5)
├── test_core/           # Core module tests (P6)
└── test_devices/        # Device module tests (P6)
```

## Coverage Target

80% test coverage for all modules.
