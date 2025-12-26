"""
USD Protocol - Message Definitions

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

Defines message IDs, structures, and constants for USD protocol.
This file mirrors the C version in common/protocol/messages.h
"""

from enum import IntEnum
from dataclasses import dataclass
from typing import Optional
import struct

# =============================================================================
# Protocol Constants
# =============================================================================

MSG_MAX_PAYLOAD_SIZE = 250
MSG_HEADER_SIZE = 2  # MsgID (1) + Length (1)
MSG_CRC_SIZE = 2
MSG_MAX_SIZE = MSG_HEADER_SIZE + MSG_MAX_PAYLOAD_SIZE + MSG_CRC_SIZE

# Response bit (OR'd with request ID)
MSG_ID_RESPONSE_BIT = 0x80


# =============================================================================
# Message ID Enums
# =============================================================================

class MsgIdSystem(IntEnum):
    """System message IDs (0x00-0x0F)"""
    PING = 0x00
    VERSION = 0x01
    RESET = 0x02
    STATUS = 0x03
    NODE_INFO = 0x04
    SET_NODE_ID = 0x05


class MsgIdMotion(IntEnum):
    """Motion message IDs (0x10-0x2F)"""
    MOVE_TO = 0x10
    MOVE_VELOCITY = 0x11
    STOP = 0x12
    HOME = 0x13
    SET_POSITION = 0x14
    JOG = 0x15
    SYNC_MOVE = 0x16


class MsgIdConfig(IntEnum):
    """Configuration message IDs (0x30-0x3F)"""
    GET_PARAM = 0x30
    SET_PARAM = 0x31
    SAVE_CONFIG = 0x32
    LOAD_CONFIG = 0x33
    RESET_CONFIG = 0x34


class MsgIdTelemetry(IntEnum):
    """Telemetry message IDs (0x40-0x4F)"""
    GET_POSITION = 0x40
    GET_VELOCITY = 0x41
    GET_TEMP = 0x42
    GET_ERROR = 0x43
    STREAM_START = 0x44
    STREAM_STOP = 0x45


class ParamId(IntEnum):
    """Parameter IDs for GET_PARAM/SET_PARAM"""
    STEPS_PER_REV = 0x00
    MICROSTEPS = 0x01
    MAX_VELOCITY = 0x02
    MAX_ACCEL = 0x03
    CURRENT_RUN = 0x04
    CURRENT_HOLD = 0x05
    PID_KP = 0x10
    PID_KI = 0x11
    PID_KD = 0x12
    ENCODER_CPR = 0x20
    NODE_ID = 0x30


class ResponseStatus(IntEnum):
    """Response status codes"""
    OK = 0x00
    ERR_CRC = 0x01
    ERR_CMD = 0x02
    ERR_PARAM = 0x03
    ERR_BUSY = 0x04
    ERR_FAULT = 0x05


class ErrorFlags(IntEnum):
    """Error flags (bitfield)"""
    NONE = 0x0000
    CRC_FAIL = 0x0001
    INVALID_CMD = 0x0002
    INVALID_PARAM = 0x0004
    OVERTEMP = 0x0008
    OVERCURRENT = 0x0010
    LIMIT_HIT = 0x0020
    ESTOP = 0x0040
    COMM_TIMEOUT = 0x0080
    ENCODER_FAULT = 0x0100


# =============================================================================
# Message Structures
# =============================================================================

@dataclass
class Message:
    """Base message class"""
    msg_id: int
    payload: bytes = b""
    
    def pack(self) -> bytes:
        """Pack message into bytes (without COBS or CRC)"""
        return bytes([self.msg_id, len(self.payload)]) + self.payload
    
    @classmethod
    def unpack(cls, data: bytes) -> "Message":
        """Unpack message from bytes"""
        if len(data) < 2:
            raise ValueError("Message too short")
        msg_id = data[0]
        length = data[1]
        if len(data) < 2 + length:
            raise ValueError(f"Payload truncated: expected {length}, got {len(data) - 2}")
        return cls(msg_id=msg_id, payload=data[2:2+length])
    
    def is_response(self) -> bool:
        """Check if this is a response message"""
        return bool(self.msg_id & MSG_ID_RESPONSE_BIT)
    
    def get_request_id(self) -> int:
        """Get the original request ID (for responses)"""
        return self.msg_id & ~MSG_ID_RESPONSE_BIT


@dataclass
class MoveToCommand:
    """Move to position command payload"""
    position: int  # steps (i32)
    velocity: int  # steps/sec / 10 (u16)
    acceleration: int = 0  # steps/sec^2 / 100 (u16)
    
    def pack(self) -> bytes:
        return struct.pack("<ihh", self.position, self.velocity, self.acceleration)
    
    @classmethod
    def unpack(cls, data: bytes) -> "MoveToCommand":
        pos, vel, accel = struct.unpack("<ihh", data[:8])
        return cls(position=pos, velocity=vel, acceleration=accel)


@dataclass  
class PositionResponse:
    """Position response payload"""
    status: ResponseStatus
    position: int  # steps (i32)
    velocity: int  # steps/sec / 10 (i16)
    
    @classmethod
    def unpack(cls, data: bytes) -> "PositionResponse":
        status, pos, vel = struct.unpack("<Bih", data[:7])
        return cls(status=ResponseStatus(status), position=pos, velocity=vel)


@dataclass
class VersionResponse:
    """Version response payload"""
    status: ResponseStatus
    major: int
    minor: int
    patch: int
    
    @classmethod
    def unpack(cls, data: bytes) -> "VersionResponse":
        status, major, minor, patch = struct.unpack("<BBBB", data[:4])
        return cls(status=ResponseStatus(status), major=major, minor=minor, patch=patch)
    
    def __str__(self) -> str:
        return f"v{self.major}.{self.minor}.{self.patch}"
