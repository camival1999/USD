/**
 * @file test_crc16.cpp
 * @brief Unit tests for CRC-16 implementation
 * 
 * Copyright (c) 2025 Camilo Valencia. All rights reserved.
 * Proprietary and confidential. See LICENSE file.
 */

#include <unity.h>
#include "crc16.h"

void test_crc16_empty(void) {
    uint8_t data[] = {};
    uint16_t crc = crc16_calculate(data, 0);
    
    // CRC of empty data with init 0xFFFF should be 0xFFFF
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, crc);
}

void test_crc16_single_byte(void) {
    uint8_t data[] = {0x00};
    uint16_t crc = crc16_calculate(data, 1);
    
    // Known value for single zero byte
    TEST_ASSERT_EQUAL_HEX16(0xE1F0, crc);
}

void test_crc16_known_sequence(void) {
    // "123456789" is a standard CRC test vector
    uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint16_t crc = crc16_calculate(data, 9);
    
    // CRC-16-CCITT of "123456789" should be 0x29B1
    TEST_ASSERT_EQUAL_HEX16(0x29B1, crc);
}

void test_crc16_update(void) {
    uint8_t data1[] = {0x01, 0x02};
    uint8_t data2[] = {0x03, 0x04};
    
    // Calculate incrementally
    uint16_t crc = crc16_update(CRC16_INIT, data1, 2);
    crc = crc16_update(crc, data2, 2);
    
    // Should match full calculation
    uint8_t full[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t crc_full = crc16_calculate(full, 4);
    
    TEST_ASSERT_EQUAL_HEX16(crc_full, crc);
}

void test_crc16_verify_pass(void) {
    uint8_t data[] = {'H', 'e', 'l', 'l', 'o'};
    uint16_t expected = crc16_calculate(data, 5);
    
    int result = crc16_verify(data, 5, expected);
    TEST_ASSERT_EQUAL(1, result);
}

void test_crc16_verify_fail(void) {
    uint8_t data[] = {'H', 'e', 'l', 'l', 'o'};
    uint16_t wrong = 0x1234;
    
    int result = crc16_verify(data, 5, wrong);
    TEST_ASSERT_EQUAL(0, result);
}

void setUp(void) {
    // Run before each test
}

void tearDown(void) {
    // Run after each test
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_crc16_empty);
    RUN_TEST(test_crc16_single_byte);
    RUN_TEST(test_crc16_known_sequence);
    RUN_TEST(test_crc16_update);
    RUN_TEST(test_crc16_verify_pass);
    RUN_TEST(test_crc16_verify_fail);
    
    return UNITY_END();
}
