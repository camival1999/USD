"""
USD - Ultimate Stepper Driver

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

Python host software for precision motion control with ESP32 firmware.

Modules:
    core    - Motion planning, trajectory generation, kinematics
    comm    - Serial/USB communication with MCU
    devices - High-level axis and robot abstractions
    utils   - Configuration, logging, helpers

Example:
    >>> import usd
    >>> usd.__version__
    '0.1.0'
"""

__version__ = "0.1.0"
__author__ = "Camilo Valencia"

# Submodule imports
from . import core
from . import comm
from . import devices
from . import utils

__all__ = [
    "__version__",
    "__author__",
    "core",
    "comm", 
    "devices",
    "utils",
]
