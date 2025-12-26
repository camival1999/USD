/**
 * @file crc16.h
 * @brief CRC-16-CCITT implementation
 * 
 * Copyright (c) 2025 Camilo Valencia. All rights reserved.
 * Proprietary and confidential. See LICENSE file.
 * 
 * Polynomial: 0x1021 (x^16 + x^12 + x^5 + 1)
 * Initial value: 0xFFFF
 * 
 * This implementation is shared between firmware and mirrors the Python version.
 */

#ifndef USD_PROTOCOL_CRC16_H
#define USD_PROTOCOL_CRC16_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CRC-16-CCITT polynomial
 */
#define CRC16_POLY 0x1021

/**
 * CRC-16-CCITT initial value
 */
#define CRC16_INIT 0xFFFF

/**
 * @brief Calculate CRC-16-CCITT checksum
 * 
 * @param data   Input data buffer
 * @param length Length of input data
 * @return uint16_t CRC-16 checksum
 */
uint16_t crc16_calculate(const uint8_t* data, size_t length);

/**
 * @brief Update running CRC-16 with new data
 * 
 * @param crc    Current CRC value (use CRC16_INIT for first call)
 * @param data   Input data buffer
 * @param length Length of input data
 * @return uint16_t Updated CRC-16 value
 */
uint16_t crc16_update(uint16_t crc, const uint8_t* data, size_t length);

/**
 * @brief Verify CRC-16 checksum
 * 
 * @param data     Data buffer (without CRC)
 * @param length   Length of data
 * @param expected Expected CRC value
 * @return int     1 if valid, 0 if invalid
 */
int crc16_verify(const uint8_t* data, size_t length, uint16_t expected);

#ifdef __cplusplus
}
#endif

#endif // USD_PROTOCOL_CRC16_H
