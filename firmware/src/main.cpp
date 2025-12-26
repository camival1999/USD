/**
 * @file main.cpp
 * @brief USD Firmware Entry Point
 * 
 * Ultimate Stepper Driver - ESP32-S3 Firmware
 * FreeRTOS-based multi-task architecture for real-time motion control.
 * 
 * @copyright Copyright (c) 2025 Camilo Valencia. All rights reserved.
 *            Proprietary and confidential. See LICENSE file.
 * 
 * @version 0.1.0
 * @date 2025-12-26
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// =============================================================================
// Configuration
// =============================================================================

// Version info
#ifndef USD_VERSION
#define USD_VERSION "0.1.0"
#endif

// LED pin for status indication (ESP32-S3 DevKit)
#define LED_PIN 48

// Task stack sizes (in words, multiply by 4 for bytes)
#define STACK_SIZE_COMM     4096
#define STACK_SIZE_MOTION   4096
#define STACK_SIZE_SAFETY   2048
#define STACK_SIZE_SENSOR   2048
#define STACK_SIZE_SYNC     1024

// Task priorities (higher = more important)
// Range: 0 (idle) to configMAX_PRIORITIES-1
#define PRIORITY_SAFETY     5   // Highest - safety critical
#define PRIORITY_MOTION     4   // High - real-time control
#define PRIORITY_SYNC       3   // Medium-high - multi-MCU coordination
#define PRIORITY_SENSOR     2   // Medium - sensor polling
#define PRIORITY_COMM       1   // Lower - host communication

// Task loop rates (ms)
#define LOOP_RATE_SAFETY_MS     10      // 100 Hz
#define LOOP_RATE_MOTION_MS     1       // 1 kHz (can be faster via timer ISR)
#define LOOP_RATE_SYNC_MS       5       // 200 Hz
#define LOOP_RATE_SENSOR_MS     20      // 50 Hz
#define LOOP_RATE_COMM_MS       10      // 100 Hz

// =============================================================================
// Task Handles
// =============================================================================

TaskHandle_t taskHandleComm = nullptr;
TaskHandle_t taskHandleMotion = nullptr;
TaskHandle_t taskHandleSafety = nullptr;
TaskHandle_t taskHandleSensor = nullptr;
TaskHandle_t taskHandleSync = nullptr;

// =============================================================================
// Shared Resources (Mutexes/Queues)
// =============================================================================

SemaphoreHandle_t serialMutex = nullptr;    // Protect Serial output

// =============================================================================
// Debug Logging
// =============================================================================

/**
 * @brief Thread-safe debug print
 * @param tag Module tag (e.g., "MOTION", "SAFETY")
 * @param msg Message to print
 */
void usd_log(const char* tag, const char* msg) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        Serial.printf("[%s] %s\n", tag, msg);
        xSemaphoreGive(serialMutex);
    }
}

/**
 * @brief Thread-safe formatted debug print
 */
void usd_logf(const char* tag, const char* fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    usd_log(tag, buffer);
}

// =============================================================================
// Task Implementations
// =============================================================================

/**
 * @brief Communication Task - Host protocol handling
 * 
 * Handles USB-CDC communication with host:
 * - Receives commands (COBS-framed)
 * - Sends responses and telemetry
 * - Manages command queue
 */
void taskComm(void* pvParameters) {
    (void)pvParameters;
    usd_log("COMM", "Task started");
    
    TickType_t lastWake = xTaskGetTickCount();
    
    for (;;) {
        // TODO: Process incoming commands
        // TODO: Send pending telemetry
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LOOP_RATE_COMM_MS));
    }
}

/**
 * @brief Motion Task - Real-time motor control
 * 
 * Highest-rate control loop:
 * - Trajectory interpolation
 * - Step pulse generation (or delegates to MCPWM)
 * - Position/velocity loop execution
 */
void taskMotion(void* pvParameters) {
    (void)pvParameters;
    usd_log("MOTION", "Task started");
    
    TickType_t lastWake = xTaskGetTickCount();
    
    for (;;) {
        // TODO: Execute motion profile step
        // TODO: Update position tracking
        // TODO: PID loop iteration (when closed-loop enabled)
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LOOP_RATE_MOTION_MS));
    }
}

/**
 * @brief Safety Task - Fault monitoring and protection
 * 
 * Monitors system health:
 * - Watchdog feeding
 * - Thermal monitoring
 * - Limit switch checking
 * - Emergency stop handling
 */
void taskSafety(void* pvParameters) {
    (void)pvParameters;
    usd_log("SAFETY", "Task started");
    
    TickType_t lastWake = xTaskGetTickCount();
    static bool ledState = false;
    
    for (;;) {
        // Heartbeat LED
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        
        // TODO: Check thermal limits
        // TODO: Check limit switches
        // TODO: Feed watchdog
        // TODO: Monitor task health
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LOOP_RATE_SAFETY_MS * 50)); // Blink at 1Hz
    }
}

/**
 * @brief Sensor Task - Encoder and feedback reading
 * 
 * Polls sensors:
 * - AS5600 magnetic encoder (I2C)
 * - Temperature sensors
 * - IMU (if present)
 */
void taskSensor(void* pvParameters) {
    (void)pvParameters;
    usd_log("SENSOR", "Task started");
    
    TickType_t lastWake = xTaskGetTickCount();
    
    for (;;) {
        // TODO: Read encoder position
        // TODO: Read temperature
        // TODO: Publish to shared state
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LOOP_RATE_SENSOR_MS));
    }
}

/**
 * @brief Sync Task - Multi-MCU coordination
 * 
 * Handles distributed network:
 * - GPIO sync pulse generation/reception
 * - Node discovery (future)
 * - Coordinated motion start
 */
void taskSync(void* pvParameters) {
    (void)pvParameters;
    usd_log("SYNC", "Task started");
    
    TickType_t lastWake = xTaskGetTickCount();
    
    for (;;) {
        // TODO: Check/generate sync signals
        // TODO: Handle multi-MCU coordination
        
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(LOOP_RATE_SYNC_MS));
    }
}

// =============================================================================
// Setup
// =============================================================================

void setup() {
    // Initialize serial for debugging (USB-CDC)
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        // Wait up to 3 seconds for USB-CDC connection
    }
    
    // Create mutex for serial access
    serialMutex = xSemaphoreCreateMutex();
    
    // Print startup banner
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║        USD - Ultimate Stepper Driver                  ║");
    Serial.println("║        Version: " USD_VERSION "                                  ║");
    Serial.println("║        Platform: ESP32-S3 @ 240MHz                    ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    
    // -------------------------------------------------------------------------
    // Create FreeRTOS Tasks
    // -------------------------------------------------------------------------
    
    Serial.println("[INIT] Creating FreeRTOS tasks...");
    
    BaseType_t result;
    
    // Safety task (highest priority)
    result = xTaskCreatePinnedToCore(
        taskSafety,
        "safety",
        STACK_SIZE_SAFETY,
        nullptr,
        PRIORITY_SAFETY,
        &taskHandleSafety,
        1   // Run on Core 1 (app core)
    );
    Serial.printf("[INIT] Safety task: %s\n", result == pdPASS ? "OK" : "FAILED");
    
    // Motion task (high priority, Core 1 for real-time)
    result = xTaskCreatePinnedToCore(
        taskMotion,
        "motion",
        STACK_SIZE_MOTION,
        nullptr,
        PRIORITY_MOTION,
        &taskHandleMotion,
        1   // Run on Core 1 (app core)
    );
    Serial.printf("[INIT] Motion task: %s\n", result == pdPASS ? "OK" : "FAILED");
    
    // Sync task (medium-high priority)
    result = xTaskCreatePinnedToCore(
        taskSync,
        "sync",
        STACK_SIZE_SYNC,
        nullptr,
        PRIORITY_SYNC,
        &taskHandleSync,
        1   // Run on Core 1
    );
    Serial.printf("[INIT] Sync task: %s\n", result == pdPASS ? "OK" : "FAILED");
    
    // Sensor task (medium priority)
    result = xTaskCreatePinnedToCore(
        taskSensor,
        "sensor",
        STACK_SIZE_SENSOR,
        nullptr,
        PRIORITY_SENSOR,
        &taskHandleSensor,
        0   // Run on Core 0 (I2C/SPI not time-critical)
    );
    Serial.printf("[INIT] Sensor task: %s\n", result == pdPASS ? "OK" : "FAILED");
    
    // Communication task (lower priority, Core 0)
    result = xTaskCreatePinnedToCore(
        taskComm,
        "comm",
        STACK_SIZE_COMM,
        nullptr,
        PRIORITY_COMM,
        &taskHandleComm,
        0   // Run on Core 0 (USB handled here)
    );
    Serial.printf("[INIT] Comm task: %s\n", result == pdPASS ? "OK" : "FAILED");
    
    // -------------------------------------------------------------------------
    // Initialization Complete
    // -------------------------------------------------------------------------
    
    Serial.println();
    Serial.println("[INIT] ═══════════════════════════════════════════════════");
    Serial.println("[INIT] USD firmware initialized successfully");
    Serial.println("[INIT] FreeRTOS scheduler running");
    Serial.println("[INIT] ═══════════════════════════════════════════════════");
    Serial.println();
}

// =============================================================================
// Main Loop (runs as lowest priority task)
// =============================================================================

void loop() {
    // Arduino loop() runs as a FreeRTOS task with priority 1
    // We use this for non-time-critical background work
    
    static uint32_t lastStats = 0;
    
    // Print task stats every 10 seconds (debug)
    #if USD_DEBUG
    if (millis() - lastStats >= 10000) {
        lastStats = millis();
        
        if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            Serial.println("\n[DEBUG] Task Statistics:");
            Serial.printf("  Free heap: %d bytes\n", ESP.getFreeHeap());
            Serial.printf("  Min free heap: %d bytes\n", ESP.getMinFreeHeap());
            
            // Task stack high water marks
            if (taskHandleMotion) {
                Serial.printf("  Motion stack free: %d words\n", 
                    uxTaskGetStackHighWaterMark(taskHandleMotion));
            }
            if (taskHandleComm) {
                Serial.printf("  Comm stack free: %d words\n", 
                    uxTaskGetStackHighWaterMark(taskHandleComm));
            }
            Serial.println();
            xSemaphoreGive(serialMutex);
        }
    }
    #endif
    
    // Yield to other tasks
    vTaskDelay(pdMS_TO_TICKS(100));
}