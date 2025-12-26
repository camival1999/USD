/**
 * @file test_cobs.cpp
 * @brief Unit tests for COBS implementation
 * 
 * Copyright (c) 2025 Camilo Valencia. All rights reserved.
 * Proprietary and confidential. See LICENSE file.
 */

#include <unity.h>
#include "cobs.h"

void test_cobs_encode_empty(void) {
    uint8_t input[] = {};
    uint8_t output[10];
    
    size_t len = cobs_encode(input, 0, output);
    TEST_ASSERT_EQUAL(0, len);  // Empty input returns 0
}

void test_cobs_encode_no_zeros(void) {
    uint8_t input[] = {0x01, 0x02, 0x03};
    uint8_t output[10];
    
    size_t len = cobs_encode(input, 3, output);
    
    // Expected: [0x04, 0x01, 0x02, 0x03]
    TEST_ASSERT_EQUAL(4, len);
    TEST_ASSERT_EQUAL(0x04, output[0]);  // Code: 4 bytes until end
    TEST_ASSERT_EQUAL(0x01, output[1]);
    TEST_ASSERT_EQUAL(0x02, output[2]);
    TEST_ASSERT_EQUAL(0x03, output[3]);
}

void test_cobs_encode_with_zero(void) {
    uint8_t input[] = {0x00};
    uint8_t output[10];
    
    size_t len = cobs_encode(input, 1, output);
    
    // Expected: [0x01, 0x01]
    TEST_ASSERT_EQUAL(2, len);
    TEST_ASSERT_EQUAL(0x01, output[0]);  // Zero at position 0
    TEST_ASSERT_EQUAL(0x01, output[1]);  // End marker
}

void test_cobs_encode_mixed(void) {
    uint8_t input[] = {0x01, 0x00, 0x02};
    uint8_t output[10];
    
    size_t len = cobs_encode(input, 3, output);
    
    // Expected: [0x02, 0x01, 0x02, 0x02]
    TEST_ASSERT_EQUAL(4, len);
    TEST_ASSERT_EQUAL(0x02, output[0]);  // 2 bytes until zero
    TEST_ASSERT_EQUAL(0x01, output[1]);
    TEST_ASSERT_EQUAL(0x02, output[2]);  // 2 bytes until end
    TEST_ASSERT_EQUAL(0x02, output[3]);
}

void test_cobs_decode_no_zeros(void) {
    uint8_t input[] = {0x04, 0x01, 0x02, 0x03};
    uint8_t output[10];
    
    size_t len = cobs_decode(input, 4, output);
    
    TEST_ASSERT_EQUAL(3, len);
    TEST_ASSERT_EQUAL(0x01, output[0]);
    TEST_ASSERT_EQUAL(0x02, output[1]);
    TEST_ASSERT_EQUAL(0x03, output[2]);
}

void test_cobs_decode_with_zero(void) {
    uint8_t input[] = {0x01, 0x01};
    uint8_t output[10];
    
    size_t len = cobs_decode(input, 2, output);
    
    TEST_ASSERT_EQUAL(1, len);
    TEST_ASSERT_EQUAL(0x00, output[0]);
}

void test_cobs_roundtrip(void) {
    uint8_t original[] = {0x45, 0x00, 0x00, 0x2C, 0x4C, 0x79, 0x00, 0x05};
    uint8_t encoded[20];
    uint8_t decoded[20];
    
    size_t enc_len = cobs_encode(original, 8, encoded);
    size_t dec_len = cobs_decode(encoded, enc_len, decoded);
    
    TEST_ASSERT_EQUAL(8, dec_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(original, decoded, 8);
}

void setUp(void) {
    // Run before each test
}

void tearDown(void) {
    // Run after each test
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_cobs_encode_empty);
    RUN_TEST(test_cobs_encode_no_zeros);
    RUN_TEST(test_cobs_encode_with_zero);
    RUN_TEST(test_cobs_encode_mixed);
    RUN_TEST(test_cobs_decode_no_zeros);
    RUN_TEST(test_cobs_decode_with_zero);
    RUN_TEST(test_cobs_roundtrip);
    
    return UNITY_END();
}
