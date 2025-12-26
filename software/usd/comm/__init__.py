"""
USD Communication Module

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

Serial/USB communication with ESP32 MCU nodes.
Implements COBS framing and CRC-16 validation.

Components:
    cobs        - COBS framing (encode/decode)
    crc16       - CRC-16-CCITT checksum
    messages    - Message definitions and structures
    protocol    - High-level protocol handler (planned)
    transport   - Serial port abstraction (planned)
"""

from . import cobs
from . import crc16
from .messages import (
    Message,
    MsgIdSystem,
    MsgIdMotion,
    MsgIdConfig,
    MsgIdTelemetry,
    ParamId,
    ResponseStatus,
    ErrorFlags,
    MoveToCommand,
    PositionResponse,
    VersionResponse,
    MSG_MAX_PAYLOAD_SIZE,
    MSG_ID_RESPONSE_BIT,
)

__all__ = [
    # Modules
    "cobs",
    "crc16",
    # Message classes
    "Message",
    "MoveToCommand",
    "PositionResponse",
    "VersionResponse",
    # Enums
    "MsgIdSystem",
    "MsgIdMotion",
    "MsgIdConfig",
    "MsgIdTelemetry",
    "ParamId",
    "ResponseStatus",
    "ErrorFlags",
    # Constants
    "MSG_MAX_PAYLOAD_SIZE",
    "MSG_ID_RESPONSE_BIT",
]
