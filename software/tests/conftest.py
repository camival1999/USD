"""
USD Test Suite

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

Pytest configuration and shared fixtures for USD tests.
"""

import pytest


@pytest.fixture
def sample_axis_config():
    """Fixture providing a sample axis configuration."""
    return {
        "name": "test_axis",
        "steps_per_rev": 200,
        "microsteps": 16,
        "max_velocity": 1000.0,  # steps/sec
        "max_acceleration": 5000.0,  # steps/sec^2
        "units": "mm",
        "units_per_rev": 8.0,  # 8mm per revolution (lead screw)
    }


@pytest.fixture
def mock_serial_port(mocker):
    """Fixture providing a mock serial port for communication tests."""
    # Will be implemented when testing comm module
    pass
