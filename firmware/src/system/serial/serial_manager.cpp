#include "serial_manager.h"
#include "../../hal/xiao_esp32s3_constants.h"
#include <stdarg.h>

// Static member definitions
bool SerialManager::initialized = false;
LogLevel SerialManager::current_log_level = LOG_LEVEL_INFO;
unsigned long SerialManager::last_performance_report = 0;
unsigned long SerialManager::total_bytes_sent = 0;
unsigned long SerialManager::message_count = 0;

// ===================================================================
// INITIALIZATION AND CONFIGURATION
// ===================================================================

bool SerialManager::initialize(unsigned long baud_rate) {
    if (initialized) {
        return true;
    }
    
    Serial.begin(baud_rate);
    
    // Wait for serial to be ready
    unsigned long start_time = millis();
    while (!Serial && (millis() - start_time) < SERIAL_TIMEOUT_MS) {
        delay(10);
    }
    
    initialized = true;
    total_bytes_sent = 0;
    message_count = 0;
    last_performance_report = millis();
    
    // Print initialization message
    printHeader("Serial Manager Initialized");
    infof(MODULE_SYSTEM, "Baud rate: %lu", baud_rate);
    infof(MODULE_SYSTEM, "Log level: %s", getLogLevelString(current_log_level));
    printSeparator();
    
    return true;
}

void SerialManager::cleanup() {
    if (!initialized) return;
    
    info("Serial Manager shutting down", MODULE_SYSTEM);
    Serial.end();
    initialized = false;
}

void SerialManager::setLogLevel(LogLevel level) {
    current_log_level = level;
    infof(MODULE_SYSTEM, "Log level set to: %s", getLogLevelString(level));
}

LogLevel SerialManager::getLogLevel() {
    return current_log_level;
}

bool SerialManager::isInitialized() {
    return initialized;
}

// ===================================================================
// PRIVATE HELPER METHODS
// ===================================================================

void SerialManager::printLogPrefix(LogLevel level, const char* module) {
    if (!initialized) return;
    
    // Timestamp
    unsigned long timestamp = millis();
    Serial.printf("[%8lu] ", timestamp);
    
    // Log level
    const char* level_str = getLogLevelString(level);
    Serial.printf("[%s] ", level_str);
    
    // Module
    if (module) {
        Serial.printf("[%s] ", module);
    }
}

const char* SerialManager::getLogLevelString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_NONE: return "NONE";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARNING: return "WARN";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_VERBOSE: return "VERBOSE";
        default: return "UNKNOWN";
    }
}

// ===================================================================
// BASIC LOGGING METHODS
// ===================================================================

void SerialManager::log(LogLevel level, const char* message, const char* module) {
    if (!initialized || level > current_log_level) return;
    
    printLogPrefix(level, module);
    Serial.println(message);
    
    message_count++;
    total_bytes_sent += strlen(message) + 20; // Approximate overhead
}

void SerialManager::log(LogLevel level, const String& message, const char* module) {
    log(level, message.c_str(), module);
}

void SerialManager::logf(LogLevel level, const char* module, const char* format, ...) {
    if (!initialized || level > current_log_level) return;
    
    va_list args;
    va_start(args, format);
    
    printLogPrefix(level, module);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
    
    va_end(args);
    
    message_count++;
    total_bytes_sent += strlen(buffer) + 20; // Approximate overhead
}

// ===================================================================
// CONVENIENCE LOGGING METHODS
// ===================================================================

void SerialManager::error(const char* message, const char* module) {
    log(LOG_LEVEL_ERROR, message, module);
}

void SerialManager::warning(const char* message, const char* module) {
    log(LOG_LEVEL_WARNING, message, module);
}

void SerialManager::info(const char* message, const char* module) {
    log(LOG_LEVEL_INFO, message, module);
}

void SerialManager::debug(const char* message, const char* module) {
    log(LOG_LEVEL_DEBUG, message, module);
}

void SerialManager::verbose(const char* message, const char* module) {
    log(LOG_LEVEL_VERBOSE, message, module);
}

// ===================================================================
// FORMATTED LOGGING METHODS
// ===================================================================

void SerialManager::errorf(const char* module, const char* format, ...) {
    if (!initialized || LOG_LEVEL_ERROR > current_log_level) return;
    
    va_list args;
    va_start(args, format);
    
    printLogPrefix(LOG_LEVEL_ERROR, module);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
    
    va_end(args);
    
    message_count++;
    total_bytes_sent += strlen(buffer) + 20;
}

void SerialManager::warningf(const char* module, const char* format, ...) {
    if (!initialized || LOG_LEVEL_WARNING > current_log_level) return;
    
    va_list args;
    va_start(args, format);
    
    printLogPrefix(LOG_LEVEL_WARNING, module);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
    
    va_end(args);
    
    message_count++;
    total_bytes_sent += strlen(buffer) + 20;
}

void SerialManager::infof(const char* module, const char* format, ...) {
    if (!initialized || LOG_LEVEL_INFO > current_log_level) return;
    
    va_list args;
    va_start(args, format);
    
    printLogPrefix(LOG_LEVEL_INFO, module);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
    
    va_end(args);
    
    message_count++;
    total_bytes_sent += strlen(buffer) + 20;
}

void SerialManager::debugf(const char* module, const char* format, ...) {
    if (!initialized || LOG_LEVEL_DEBUG > current_log_level) return;
    
    va_list args;
    va_start(args, format);
    
    printLogPrefix(LOG_LEVEL_DEBUG, module);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
    
    va_end(args);
    
    message_count++;
    total_bytes_sent += strlen(buffer) + 20;
}

void SerialManager::verbosef(const char* module, const char* format, ...) {
    if (!initialized || LOG_LEVEL_VERBOSE > current_log_level) return;
    
    va_list args;
    va_start(args, format);
    
    printLogPrefix(LOG_LEVEL_VERBOSE, module);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
    
    va_end(args);
    
    message_count++;
    total_bytes_sent += strlen(buffer) + 20;
}

// ===================================================================
// PERFORMANCE MONITORING
// ===================================================================

void SerialManager::reportPerformance() {
    if (!initialized) return;
    
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - last_performance_report;
    
    if (elapsed >= 30000) { // Report every 30 seconds
        infof(MODULE_SYSTEM, "Performance: %lu messages, %lu bytes in %lu ms", 
              message_count, total_bytes_sent, elapsed);
        last_performance_report = current_time;
    }
}

void SerialManager::logPerformanceMetrics(const char* operation, unsigned long duration_ms) {
    debugf(MODULE_SYSTEM, "Performance: %s took %lu ms", operation, duration_ms);
}

// ===================================================================
// DATA LOGGING
// ===================================================================

void SerialManager::logHexData(const char* label, const uint8_t* data, size_t length, const char* module) {
    if (!initialized || LOG_LEVEL_DEBUG > current_log_level) return;
    
    printLogPrefix(LOG_LEVEL_DEBUG, module);
    Serial.printf("%s (%zu bytes): ", label, length);
    
    for (size_t i = 0; i < length && i < 32; i++) { // Limit to 32 bytes
        Serial.printf("%02X ", data[i]);
    }
    
    if (length > 32) {
        Serial.print("...");
    }
    
    Serial.println();
    message_count++;
    total_bytes_sent += length * 3 + 50; // Approximate
}

void SerialManager::logAudioData(size_t bytes_read, const char* module) {
    debugf(module, "Audio data: %zu bytes", bytes_read);
}

void SerialManager::logBatteryData(float voltage, uint8_t level, bool charging, const char* module) {
    infof(module, "Battery: %.2fV, %d%%, %s", voltage, level, charging ? "charging" : "discharging");
}

void SerialManager::logMemoryData(size_t free_heap, size_t free_psram, const char* module) {
    infof(module, "Memory: Heap=%zu KB, PSRAM=%zu KB", free_heap / 1024, free_psram / 1024);
}

// ===================================================================
// SYSTEM STATUS LOGGING
// ===================================================================

void SerialManager::logSystemStatus(const char* component, const char* status, const char* module) {
    infof(module ? module : MODULE_SYSTEM, "%s: %s", component, status);
}

void SerialManager::logInitialization(const char* component, bool success, const char* module) {
    if (success) {
        infof(module ? module : MODULE_SYSTEM, "✅ %s initialized successfully", component);
    } else {
        errorf(module ? module : MODULE_SYSTEM, "❌ %s initialization failed", component);
    }
}

void SerialManager::logError(const char* component, const char* error_msg, const char* module) {
    errorf(module ? module : MODULE_SYSTEM, "%s error: %s", component, error_msg);
}

// ===================================================================
// BLE SPECIFIC LOGGING
// ===================================================================

void SerialManager::logBLEEvent(const char* event, const char* details) {
    if (details) {
        infof(MODULE_BLE, "%s: %s", event, details);
    } else {
        infof(MODULE_BLE, "%s", event);
    }
}

void SerialManager::logBLEData(const char* characteristic, size_t bytes, const char* direction) {
    debugf(MODULE_BLE, "%s %s: %zu bytes", direction, characteristic, bytes);
}

void SerialManager::logBLEConnection(bool connected, const char* device_name) {
    if (connected) {
        if (device_name) {
            infof(MODULE_BLE, "Connected to device: %s", device_name);
        } else {
            info("BLE connection established", MODULE_BLE);
        }
    } else {
        info("BLE connection lost", MODULE_BLE);
    }
}

// ===================================================================
// CAMERA SPECIFIC LOGGING
// ===================================================================

void SerialManager::logCameraEvent(const char* event, const char* details) {
    if (details) {
        infof(MODULE_CAMERA, "%s: %s", event, details);
    } else {
        infof(MODULE_CAMERA, "%s", event);
    }
}

void SerialManager::logPhotoCapture(size_t photo_size, const char* quality) {
    if (quality) {
        infof(MODULE_CAMERA, "Photo captured: %zu bytes, quality: %s", photo_size, quality);
    } else {
        infof(MODULE_CAMERA, "Photo captured: %zu bytes", photo_size);
    }
}

void SerialManager::logVideoStream(uint8_t fps, uint32_t frame_count) {
    debugf(MODULE_CAMERA, "Video stream: %d fps, frame #%lu", fps, frame_count);
}

// ===================================================================
// MICROPHONE SPECIFIC LOGGING
// ===================================================================

void SerialManager::logMicrophoneEvent(const char* event, const char* details) {
    if (details) {
        infof(MODULE_MICROPHONE, "%s: %s", event, details);
    } else {
        infof(MODULE_MICROPHONE, "%s", event);
    }
}

void SerialManager::logAudioProcessing(size_t input_bytes, size_t output_bytes, const char* codec) {
    if (codec) {
        debugf(MODULE_MICROPHONE, "Audio processing (%s): %zu -> %zu bytes", codec, input_bytes, output_bytes);
    } else {
        debugf(MODULE_MICROPHONE, "Audio processing: %zu -> %zu bytes", input_bytes, output_bytes);
    }
}

// ===================================================================
// POWER MANAGEMENT LOGGING
// ===================================================================

void SerialManager::logPowerEvent(const char* event, const char* details) {
    if (details) {
        infof(MODULE_POWER, "%s: %s", event, details);
    } else {
        infof(MODULE_POWER, "%s", event);
    }
}

void SerialManager::logPowerMode(const char* mode, uint32_t cpu_freq_mhz) {
    infof(MODULE_POWER, "Power mode: %s, CPU: %lu MHz", mode, cpu_freq_mhz);
}

// ===================================================================
// FORMATTING AND DISPLAY
// ===================================================================

void SerialManager::printSeparator(char character, int length) {
    if (!initialized) return;
    
    for (int i = 0; i < length; i++) {
        Serial.print(character);
    }
    Serial.println();
    
    message_count++;
    total_bytes_sent += length + 1;
}

void SerialManager::printHeader(const char* title) {
    if (!initialized) return;
    
    int title_len = strlen(title);
    int padding = (60 - title_len) / 2;
    
    printSeparator('=', 60);
    
    for (int i = 0; i < padding; i++) {
        Serial.print(' ');
    }
    Serial.println(title);
    
    printSeparator('=', 60);
    
    message_count += 3;
    total_bytes_sent += 60 * 2 + title_len + padding + 3;
}

void SerialManager::printSubheader(const char* subtitle) {
    if (!initialized) return;
    
    Serial.println(subtitle);
    printSeparator('-', strlen(subtitle));
    
    message_count += 2;
    total_bytes_sent += strlen(subtitle) * 2 + 2;
}

// ===================================================================
// RAW SERIAL ACCESS
// ===================================================================

void SerialManager::print(const char* message) {
    if (!initialized) return;
    
    Serial.print(message);
    total_bytes_sent += strlen(message);
}

void SerialManager::println(const char* message) {
    if (!initialized) return;
    
    Serial.println(message);
    message_count++;
    total_bytes_sent += strlen(message) + 1;
}

void SerialManager::printf(const char* format, ...) {
    if (!initialized) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.print(buffer);
    
    va_end(args);
    
    total_bytes_sent += strlen(buffer);
} 