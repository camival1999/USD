"""
USD Protocol Tests - COBS and CRC

Copyright (c) 2025 Camilo Valencia. All rights reserved.
Proprietary and confidential. See LICENSE file.

Tests for the protocol layer: COBS framing and CRC-16 checksums.
"""

import pytest
from usd.comm import cobs, crc16
from usd.comm.messages import Message, MsgIdSystem, MoveToCommand


class TestCOBS:
    """COBS encoding/decoding tests."""
    
    def test_encode_no_zeros(self):
        """Test encoding data without zeros."""
        data = bytes([0x01, 0x02, 0x03])
        encoded = cobs.encode(data)
        
        # Expected: [0x04, 0x01, 0x02, 0x03]
        assert encoded == bytes([0x04, 0x01, 0x02, 0x03])
    
    def test_encode_single_zero(self):
        """Test encoding a single zero byte."""
        data = bytes([0x00])
        encoded = cobs.encode(data)
        
        # Expected: [0x01, 0x01]
        assert encoded == bytes([0x01, 0x01])
    
    def test_encode_mixed(self):
        """Test encoding mixed data with zeros."""
        data = bytes([0x01, 0x00, 0x02])
        encoded = cobs.encode(data)
        
        # Expected: [0x02, 0x01, 0x02, 0x02]
        assert encoded == bytes([0x02, 0x01, 0x02, 0x02])
    
    def test_decode_no_zeros(self):
        """Test decoding data without zeros."""
        encoded = bytes([0x04, 0x01, 0x02, 0x03])
        decoded = cobs.decode(encoded)
        
        assert decoded == bytes([0x01, 0x02, 0x03])
    
    def test_decode_with_zero(self):
        """Test decoding data that had zeros."""
        encoded = bytes([0x01, 0x01])
        decoded = cobs.decode(encoded)
        
        assert decoded == bytes([0x00])
    
    def test_roundtrip(self):
        """Test encode/decode roundtrip."""
        original = bytes([0x45, 0x00, 0x00, 0x2C, 0x4C, 0x79, 0x00, 0x05])
        encoded = cobs.encode(original)
        decoded = cobs.decode(encoded)
        
        assert decoded == original
    
    def test_frame_unframe(self):
        """Test frame/unframe with delimiter."""
        data = bytes([0x01, 0x02, 0x03])
        framed = cobs.frame(data)
        
        # Should end with delimiter
        assert framed[-1] == 0x00
        
        # Unframe should restore original
        unframed = cobs.unframe(framed)
        assert unframed == data
    
    def test_decode_invalid_zero_in_data(self):
        """Test that zero code byte raises error."""
        # A zero appearing as a code byte should raise an error
        # [0x03, 0x01, 0x02, 0x00] - after first block, code is 0x00
        with pytest.raises(ValueError, match="zero byte"):
            cobs.decode(bytes([0x03, 0x01, 0x02, 0x00]))
    
    def test_encode_max_size(self):
        """Test encoding at maximum size."""
        data = bytes([0xFF] * 250)
        encoded = cobs.encode(data)
        
        # Should succeed
        assert len(encoded) <= cobs.COBS_MAX_OUTPUT_SIZE
    
    def test_encode_too_large(self):
        """Test that oversized data raises error."""
        data = bytes([0xFF] * 251)
        with pytest.raises(ValueError, match="too large"):
            cobs.encode(data)


class TestCRC16:
    """CRC-16-CCITT tests."""
    
    def test_empty(self):
        """Test CRC of empty data."""
        crc = crc16.calculate(b"")
        assert crc == 0xFFFF  # Init value
    
    def test_known_sequence(self):
        """Test CRC with known test vector."""
        # "123456789" is a standard CRC test vector
        data = b"123456789"
        crc = crc16.calculate(data)
        
        # CRC-16-CCITT of "123456789" should be 0x29B1
        assert crc == 0x29B1
    
    def test_update_incremental(self):
        """Test incremental CRC calculation."""
        data1 = bytes([0x01, 0x02])
        data2 = bytes([0x03, 0x04])
        
        # Calculate incrementally
        crc = crc16.update(crc16.CRC16_INIT, data1)
        crc = crc16.update(crc, data2)
        
        # Should match full calculation
        full = bytes([0x01, 0x02, 0x03, 0x04])
        crc_full = crc16.calculate(full)
        
        assert crc == crc_full
    
    def test_verify_pass(self):
        """Test CRC verification with correct value."""
        data = b"Hello"
        expected = crc16.calculate(data)
        
        assert crc16.verify(data, expected) is True
    
    def test_verify_fail(self):
        """Test CRC verification with wrong value."""
        data = b"Hello"
        wrong = 0x1234
        
        assert crc16.verify(data, wrong) is False
    
    def test_append(self):
        """Test appending CRC to data."""
        data = b"Test"
        with_crc = crc16.append(data)
        
        # Should be 2 bytes longer
        assert len(with_crc) == len(data) + 2
        
        # Last 2 bytes should be CRC (little-endian)
        expected_crc = crc16.calculate(data)
        assert with_crc[-2] == (expected_crc & 0xFF)
        assert with_crc[-1] == ((expected_crc >> 8) & 0xFF)
    
    def test_check_and_strip(self):
        """Test verifying and removing CRC."""
        original = b"Test"
        with_crc = crc16.append(original)
        
        stripped = crc16.check_and_strip(with_crc)
        assert stripped == original
    
    def test_check_and_strip_corrupted(self):
        """Test that corrupted data raises error."""
        original = b"Test"
        with_crc = crc16.append(original)
        
        # Corrupt one byte
        corrupted = bytes([with_crc[0] ^ 0xFF]) + with_crc[1:]
        
        with pytest.raises(ValueError, match="CRC mismatch"):
            crc16.check_and_strip(corrupted)


class TestMessages:
    """Message structure tests."""
    
    def test_message_pack_unpack(self):
        """Test basic message packing."""
        msg = Message(msg_id=0x01, payload=bytes([0x05, 0x06]))
        packed = msg.pack()
        
        # [msg_id, length, payload...]
        assert packed == bytes([0x01, 0x02, 0x05, 0x06])
        
        # Unpack
        unpacked = Message.unpack(packed)
        assert unpacked.msg_id == 0x01
        assert unpacked.payload == bytes([0x05, 0x06])
    
    def test_is_response(self):
        """Test response detection."""
        request = Message(msg_id=0x00)
        response = Message(msg_id=0x80)
        
        assert request.is_response() is False
        assert response.is_response() is True
    
    def test_get_request_id(self):
        """Test extracting request ID from response."""
        response = Message(msg_id=0x90)
        
        # 0x90 = 0x80 | 0x10
        assert response.get_request_id() == 0x10
    
    def test_move_to_command(self):
        """Test MoveToCommand packing."""
        cmd = MoveToCommand(position=1000, velocity=100, acceleration=50)
        packed = cmd.pack()
        
        # Should be 8 bytes: i32 + i16 + i16
        assert len(packed) == 8
        
        # Unpack and verify
        unpacked = MoveToCommand.unpack(packed)
        assert unpacked.position == 1000
        assert unpacked.velocity == 100
        assert unpacked.acceleration == 50
