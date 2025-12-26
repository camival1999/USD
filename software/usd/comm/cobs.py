"""
USD Protocol - COBS Implementation

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

COBS (Consistent Overhead Byte Stuffing) encodes data to eliminate zero bytes,
using 0x00 as a packet delimiter.

This implementation mirrors the C version in common/protocol/cobs.h
"""

# Constants (match cobs.h)
COBS_MAX_INPUT_SIZE = 250
COBS_MAX_OUTPUT_SIZE = COBS_MAX_INPUT_SIZE + 2
COBS_DELIMITER = 0x00


def encode(data: bytes) -> bytes:
    """
    Encode data using COBS.
    
    Args:
        data: Input data (max 250 bytes)
        
    Returns:
        COBS-encoded data (without trailing delimiter)
        
    Raises:
        ValueError: If input exceeds maximum size
    """
    if len(data) > COBS_MAX_INPUT_SIZE:
        raise ValueError(f"Input too large: {len(data)} > {COBS_MAX_INPUT_SIZE}")
    
    if len(data) == 0:
        return bytes([0x01])  # Empty input encodes to single byte
    
    output = bytearray()
    code_idx = 0
    output.append(0)  # Placeholder for first code byte
    code = 1
    
    for byte in data:
        if byte == 0:
            # Found zero - write code and start new block
            output[code_idx] = code
            code_idx = len(output)
            output.append(0)  # Placeholder for next code
            code = 1
        else:
            output.append(byte)
            code += 1
            
            if code == 0xFF:
                # Max code reached - start new block
                output[code_idx] = code
                code_idx = len(output)
                output.append(0)  # Placeholder
                code = 1
    
    # Write final code
    output[code_idx] = code
    
    return bytes(output)


def decode(data: bytes) -> bytes:
    """
    Decode COBS-encoded data.
    
    Args:
        data: COBS-encoded data (without delimiter)
        
    Returns:
        Decoded data
        
    Raises:
        ValueError: If input contains invalid COBS encoding
    """
    if len(data) == 0:
        return b""
    
    output = bytearray()
    idx = 0
    
    while idx < len(data):
        code = data[idx]
        
        if code == 0:
            raise ValueError("Invalid COBS: zero byte in encoded data")
        
        idx += 1
        
        # Copy (code - 1) bytes
        for _ in range(1, code):
            if idx >= len(data):
                raise ValueError("Invalid COBS: unexpected end of data")
            output.append(data[idx])
            idx += 1
        
        # If code < 0xFF and not at end, insert zero
        if code < 0xFF and idx < len(data):
            output.append(0)
    
    return bytes(output)


def frame(data: bytes) -> bytes:
    """
    Encode data with COBS and append delimiter.
    
    Args:
        data: Input data
        
    Returns:
        COBS-encoded data with trailing 0x00 delimiter
    """
    return encode(data) + bytes([COBS_DELIMITER])


def unframe(data: bytes) -> bytes:
    """
    Remove delimiter and decode COBS data.
    
    Args:
        data: COBS-framed data with trailing delimiter
        
    Returns:
        Decoded data
        
    Raises:
        ValueError: If no delimiter found or invalid encoding
    """
    if not data or data[-1] != COBS_DELIMITER:
        raise ValueError("No COBS delimiter found")
    
    return decode(data[:-1])
