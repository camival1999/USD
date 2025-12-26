/**
 * @file cobs.h
 * @brief COBS (Consistent Overhead Byte Stuffing) implementation
 * 
 * Copyright (c) 2025 Camilo Valencia. All rights reserved.
 * Proprietary and confidential. See LICENSE file.
 * 
 * COBS encodes data to eliminate zero bytes, using 0x00 as a packet delimiter.
 * This implementation is shared between firmware and mirrors the Python version.
 * 
 * Reference: https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
 */

#ifndef USD_PROTOCOL_COBS_H
#define USD_PROTOCOL_COBS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Maximum input size for COBS encoding.
 * Output will be at most input_size + ceil(input_size/254) + 1 bytes.
 */
#define COBS_MAX_INPUT_SIZE 250

/**
 * Maximum output size after COBS encoding (including overhead).
 */
#define COBS_MAX_OUTPUT_SIZE (COBS_MAX_INPUT_SIZE + 2)

/**
 * COBS packet delimiter (zero byte).
 */
#define COBS_DELIMITER 0x00

/**
 * @brief Encode data using COBS
 * 
 * @param input     Input data buffer
 * @param input_len Length of input data (max COBS_MAX_INPUT_SIZE)
 * @param output    Output buffer (must be at least input_len + 2 bytes)
 * @return size_t   Length of encoded data, or 0 on error
 * 
 * @note Output does NOT include the trailing delimiter byte.
 *       Caller should append COBS_DELIMITER when sending.
 */
size_t cobs_encode(const uint8_t* input, size_t input_len, uint8_t* output);

/**
 * @brief Decode COBS-encoded data
 * 
 * @param input     COBS-encoded input buffer (without delimiter)
 * @param input_len Length of encoded data
 * @param output    Output buffer (must be at least input_len bytes)
 * @return size_t   Length of decoded data, or 0 on error
 */
size_t cobs_decode(const uint8_t* input, size_t input_len, uint8_t* output);

#ifdef __cplusplus
}
#endif

#endif // USD_PROTOCOL_COBS_H
