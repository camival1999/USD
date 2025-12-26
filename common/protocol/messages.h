/**
 * @file messages.h
 * @brief USD Protocol Message Definitions
 * 
 * Copyright (c) 2025 Camilo Valencia. All rights reserved.
 * Proprietary and confidential. See LICENSE file.
 * 
 * Defines message IDs, structures, and constants for USD protocol.
 * This file is the single source of truth - Python version mirrors this.
 */

#ifndef USD_PROTOCOL_MESSAGES_H
#define USD_PROTOCOL_MESSAGES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Protocol Constants
// =============================================================================

/** Maximum payload size (excluding header and CRC) */
#define MSG_MAX_PAYLOAD_SIZE 250

/** Header size: MsgID (1) + Length (1) */
#define MSG_HEADER_SIZE 2

/** CRC size */
#define MSG_CRC_SIZE 2

/** Maximum total message size */
#define MSG_MAX_SIZE (MSG_HEADER_SIZE + MSG_MAX_PAYLOAD_SIZE + MSG_CRC_SIZE)

// =============================================================================
// Message ID Ranges
// =============================================================================

/** System commands: 0x00-0x0F */
#define MSG_ID_SYSTEM_MIN       0x00
#define MSG_ID_SYSTEM_MAX       0x0F

/** Motion commands: 0x10-0x2F */
#define MSG_ID_MOTION_MIN       0x10
#define MSG_ID_MOTION_MAX       0x2F

/** Configuration commands: 0x30-0x3F */
#define MSG_ID_CONFIG_MIN       0x30
#define MSG_ID_CONFIG_MAX       0x3F

/** Telemetry requests: 0x40-0x4F */
#define MSG_ID_TELEMETRY_MIN    0x40
#define MSG_ID_TELEMETRY_MAX    0x4F

/** Responses: 0x80-0xFF (0x80 + request ID) */
#define MSG_ID_RESPONSE_BIT     0x80

// =============================================================================
// System Message IDs (0x00-0x0F)
// =============================================================================

typedef enum {
    MSG_PING            = 0x00,  /**< Ping request (payload: none) */
    MSG_VERSION         = 0x01,  /**< Get version (payload: none) */
    MSG_RESET           = 0x02,  /**< Reset MCU (payload: none) */
    MSG_STATUS          = 0x03,  /**< Get system status */
    MSG_NODE_INFO       = 0x04,  /**< Get node info (for multi-MCU) */
    MSG_SET_NODE_ID     = 0x05,  /**< Set node ID (payload: u8 node_id) */
} MsgIdSystem;

// =============================================================================
// Motion Message IDs (0x10-0x2F)
// =============================================================================

typedef enum {
    MSG_MOVE_TO         = 0x10,  /**< Move to position (payload: i32 position, u16 velocity) */
    MSG_MOVE_VELOCITY   = 0x11,  /**< Move at velocity (payload: i16 velocity) */
    MSG_STOP            = 0x12,  /**< Stop motion (payload: u8 mode: 0=decel, 1=immediate) */
    MSG_HOME            = 0x13,  /**< Home axis (payload: u8 direction, u16 velocity) */
    MSG_SET_POSITION    = 0x14,  /**< Set current position (payload: i32 position) */
    MSG_JOG             = 0x15,  /**< Jog motion (payload: i8 direction, u16 velocity) */
    MSG_SYNC_MOVE       = 0x16,  /**< Synchronized move start (multi-MCU) */
} MsgIdMotion;

// =============================================================================
// Configuration Message IDs (0x30-0x3F)
// =============================================================================

typedef enum {
    MSG_GET_PARAM       = 0x30,  /**< Get parameter (payload: u8 param_id) */
    MSG_SET_PARAM       = 0x31,  /**< Set parameter (payload: u8 param_id, value) */
    MSG_SAVE_CONFIG     = 0x32,  /**< Save config to flash */
    MSG_LOAD_CONFIG     = 0x33,  /**< Load config from flash */
    MSG_RESET_CONFIG    = 0x34,  /**< Reset to defaults */
} MsgIdConfig;

// =============================================================================
// Telemetry Message IDs (0x40-0x4F)
// =============================================================================

typedef enum {
    MSG_GET_POSITION    = 0x40,  /**< Get current position */
    MSG_GET_VELOCITY    = 0x41,  /**< Get current velocity */
    MSG_GET_TEMP        = 0x42,  /**< Get temperature readings */
    MSG_GET_ERROR       = 0x43,  /**< Get error flags */
    MSG_STREAM_START    = 0x44,  /**< Start telemetry streaming */
    MSG_STREAM_STOP     = 0x45,  /**< Stop telemetry streaming */
} MsgIdTelemetry;

// =============================================================================
// Parameter IDs (for MSG_GET_PARAM / MSG_SET_PARAM)
// =============================================================================

typedef enum {
    PARAM_STEPS_PER_REV     = 0x00,  /**< Steps per revolution (u16) */
    PARAM_MICROSTEPS        = 0x01,  /**< Microstepping divisor (u8) */
    PARAM_MAX_VELOCITY      = 0x02,  /**< Max velocity (u32 steps/sec) */
    PARAM_MAX_ACCEL         = 0x03,  /**< Max acceleration (u32 steps/sec^2) */
    PARAM_CURRENT_RUN       = 0x04,  /**< Run current (u8, 0-31) */
    PARAM_CURRENT_HOLD      = 0x05,  /**< Hold current (u8, 0-31) */
    PARAM_PID_KP            = 0x10,  /**< Position PID Kp (f32) */
    PARAM_PID_KI            = 0x11,  /**< Position PID Ki (f32) */
    PARAM_PID_KD            = 0x12,  /**< Position PID Kd (f32) */
    PARAM_ENCODER_CPR       = 0x20,  /**< Encoder counts per rev (u16) */
    PARAM_NODE_ID           = 0x30,  /**< Node ID (u8) */
} ParamId;

// =============================================================================
// Error Flags
// =============================================================================

typedef enum {
    ERR_NONE            = 0x0000,
    ERR_CRC_FAIL        = 0x0001,  /**< CRC check failed */
    ERR_INVALID_CMD     = 0x0002,  /**< Unknown command ID */
    ERR_INVALID_PARAM   = 0x0004,  /**< Invalid parameter */
    ERR_OVERTEMP        = 0x0008,  /**< Over temperature */
    ERR_OVERCURRENT     = 0x0010,  /**< Over current */
    ERR_LIMIT_HIT       = 0x0020,  /**< Limit switch triggered */
    ERR_ESTOP           = 0x0040,  /**< Emergency stop active */
    ERR_COMM_TIMEOUT    = 0x0080,  /**< Communication timeout */
    ERR_ENCODER_FAULT   = 0x0100,  /**< Encoder read error */
} ErrorFlags;

// =============================================================================
// Response Status Codes
// =============================================================================

typedef enum {
    RESP_OK             = 0x00,  /**< Command successful */
    RESP_ERR_CRC        = 0x01,  /**< CRC error */
    RESP_ERR_CMD        = 0x02,  /**< Unknown command */
    RESP_ERR_PARAM      = 0x03,  /**< Invalid parameter */
    RESP_ERR_BUSY       = 0x04,  /**< Device busy */
    RESP_ERR_FAULT      = 0x05,  /**< Fault condition active */
} ResponseStatus;

// =============================================================================
// Message Structures
// =============================================================================

/** Message header (common to all messages) */
typedef struct __attribute__((packed)) {
    uint8_t msg_id;     /**< Message ID */
    uint8_t length;     /**< Payload length */
} MsgHeader;

/** Ping response */
typedef struct __attribute__((packed)) {
    uint8_t status;     /**< Response status */
    uint8_t node_id;    /**< Node ID */
} MsgPingResponse;

/** Version response */
typedef struct __attribute__((packed)) {
    uint8_t status;     /**< Response status */
    uint8_t major;      /**< Major version */
    uint8_t minor;      /**< Minor version */
    uint8_t patch;      /**< Patch version */
} MsgVersionResponse;

/** Move to position command */
typedef struct __attribute__((packed)) {
    int32_t position;   /**< Target position (steps) */
    uint16_t velocity;  /**< Velocity (steps/sec / 10) */
    uint16_t accel;     /**< Acceleration (steps/sec^2 / 100) */
} MsgMoveTo;

/** Position response */
typedef struct __attribute__((packed)) {
    uint8_t status;     /**< Response status */
    int32_t position;   /**< Current position (steps) */
    int16_t velocity;   /**< Current velocity (steps/sec / 10) */
} MsgPositionResponse;

#ifdef __cplusplus
}
#endif

#endif // USD_PROTOCOL_MESSAGES_H
