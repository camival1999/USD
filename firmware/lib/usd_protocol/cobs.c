/**
 * @file cobs.c
 * @brief COBS (Consistent Overhead Byte Stuffing) implementation
 * 
 * Copyright (c) 2025 Camilo Valencia. All rights reserved.
 * Proprietary and confidential. See LICENSE file.
 */

#include "cobs.h"

size_t cobs_encode(const uint8_t* input, size_t input_len, uint8_t* output) {
    if (input == NULL || output == NULL || input_len == 0) {
        return 0;
    }
    
    if (input_len > COBS_MAX_INPUT_SIZE) {
        return 0;
    }
    
    size_t read_idx = 0;
    size_t write_idx = 1;  // Reserve space for first code byte
    size_t code_idx = 0;   // Index of current code byte
    uint8_t code = 1;      // Current code value (distance to next zero or end)
    
    while (read_idx < input_len) {
        if (input[read_idx] == 0) {
            // Found a zero byte - write the code
            output[code_idx] = code;
            code_idx = write_idx;
            write_idx++;
            code = 1;
        } else {
            // Non-zero byte - copy it
            output[write_idx] = input[read_idx];
            write_idx++;
            code++;
            
            // If we've reached max code value (255), need a new block
            if (code == 0xFF) {
                output[code_idx] = code;
                code_idx = write_idx;
                write_idx++;
                code = 1;
            }
        }
        read_idx++;
    }
    
    // Write final code byte
    output[code_idx] = code;
    
    return write_idx;
}

size_t cobs_decode(const uint8_t* input, size_t input_len, uint8_t* output) {
    if (input == NULL || output == NULL || input_len == 0) {
        return 0;
    }
    
    size_t read_idx = 0;
    size_t write_idx = 0;
    
    while (read_idx < input_len) {
        uint8_t code = input[read_idx];
        
        if (code == 0) {
            // Invalid: zero byte in COBS-encoded data
            return 0;
        }
        
        read_idx++;
        
        // Copy (code - 1) bytes
        for (uint8_t i = 1; i < code; i++) {
            if (read_idx >= input_len) {
                // Unexpected end of input
                return 0;
            }
            output[write_idx] = input[read_idx];
            write_idx++;
            read_idx++;
        }
        
        // If code < 0xFF and we're not at the end, insert a zero
        if (code < 0xFF && read_idx < input_len) {
            output[write_idx] = 0;
            write_idx++;
        }
    }
    
    return write_idx;
}
