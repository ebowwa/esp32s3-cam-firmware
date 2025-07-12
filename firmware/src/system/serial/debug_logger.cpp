#include "debug_logger.h"
#include <stdarg.h>

// Static member definitions
uint8_t DebugLogger::debug_categories = DEBUG_ALL;
bool DebugLogger::timing_enabled = false;
bool DebugLogger::memory_tracking_enabled = false;
bool DebugLogger::performance_monitoring_enabled = false;

// Timing tracking
TimingEntry DebugLogger::timing_entries[MAX_TIMING_ENTRIES];
uint8_t DebugLogger::timing_entry_count = 0;
unsigned long DebugLogger::total_timing_operations = 0;

// Memory tracking
MemorySample DebugLogger::memory_samples[10];
uint8_t DebugLogger::memory_sample_count = 0;
unsigned long DebugLogger::last_memory_sample = 0;

// Performance monitoring
unsigned long DebugLogger::last_performance_report = 0;
unsigned long DebugLogger::total_debug_messages = 0;
unsigned long DebugLogger::total_debug_bytes = 0;

// ===================================================================
// INITIALIZATION AND CONFIGURATION
// ===================================================================

void DebugLogger::initialize(uint8_t categories) {
    debug_categories = categories;
    timing_enabled = false;
    memory_tracking_enabled = false;
    performance_monitoring_enabled = false;
    
    // Reset counters
    timing_entry_count = 0;
    total_timing_operations = 0;
    memory_sample_count = 0;
    last_memory_sample = 0;
    last_performance_report = millis();
    total_debug_messages = 0;
    total_debug_bytes = 0;
    
    // Initialize timing entries
    for (int i = 0; i < MAX_TIMING_ENTRIES; i++) {
        timing_entries[i].operation = nullptr;
        timing_entries[i].start_time = 0;
        timing_entries[i].duration = 0;
        timing_entries[i].active = false;
    }
    
    // Initialize memory samples
    for (int i = 0; i < 10; i++) {
        memory_samples[i].timestamp = 0;
        memory_samples[i].free_heap = 0;
        memory_samples[i].free_psram = 0;
        memory_samples[i].largest_free_block = 0;
    }
    
    SerialManager::info("Debug Logger initialized", MODULE_SYSTEM);
}

void DebugLogger::cleanup() {
    if (performance_monitoring_enabled) {
        reportPerformanceMetrics();
    }
    
    SerialManager::info("Debug Logger shutting down", MODULE_SYSTEM);
}

void DebugLogger::setCategories(uint8_t categories) {
    debug_categories = categories;
    SerialManager::infof(MODULE_SYSTEM, "Debug categories set to: 0x%02X", categories);
}

uint8_t DebugLogger::getCategories() {
    return debug_categories;
}

void DebugLogger::enableTiming(bool enabled) {
    timing_enabled = enabled;
    SerialManager::infof(MODULE_SYSTEM, "Timing debug %s", enabled ? "enabled" : "disabled");
}

void DebugLogger::enableMemoryTracking(bool enabled) {
    memory_tracking_enabled = enabled;
    SerialManager::infof(MODULE_SYSTEM, "Memory tracking %s", enabled ? "enabled" : "disabled");
}

void DebugLogger::enablePerformanceMonitoring(bool enabled) {
    performance_monitoring_enabled = enabled;
    SerialManager::infof(MODULE_SYSTEM, "Performance monitoring %s", enabled ? "enabled" : "disabled");
}

// ===================================================================
// CATEGORY-BASED LOGGING
// ===================================================================

void DebugLogger::logSystem(const char* message) {
    if (debug_categories & DEBUG_SYSTEM) {
        SerialManager::debug(message, MODULE_SYSTEM);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::logBLE(const char* message) {
    if (debug_categories & DEBUG_BLE) {
        SerialManager::debug(message, MODULE_BLE);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::logCamera(const char* message) {
    if (debug_categories & DEBUG_CAMERA) {
        SerialManager::debug(message, MODULE_CAMERA);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::logMicrophone(const char* message) {
    if (debug_categories & DEBUG_MICROPHONE) {
        SerialManager::debug(message, MODULE_MICROPHONE);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::logBattery(const char* message) {
    if (debug_categories & DEBUG_BATTERY) {
        SerialManager::debug(message, MODULE_BATTERY);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::logPower(const char* message) {
    if (debug_categories & DEBUG_POWER) {
        SerialManager::debug(message, MODULE_POWER);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::logMemory(const char* message) {
    if (debug_categories & DEBUG_MEMORY) {
        SerialManager::debug(message, MODULE_MEMORY);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::logTiming(const char* message) {
    if (debug_categories & DEBUG_TIMING) {
        SerialManager::debug(message, MODULE_SYSTEM);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

// ===================================================================
// FORMATTED CATEGORY LOGGING
// ===================================================================

void DebugLogger::logSystemf(const char* format, ...) {
    if (!(debug_categories & DEBUG_SYSTEM)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_SYSTEM);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

void DebugLogger::logBLEf(const char* format, ...) {
    if (!(debug_categories & DEBUG_BLE)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_BLE);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

void DebugLogger::logCameraf(const char* format, ...) {
    if (!(debug_categories & DEBUG_CAMERA)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_CAMERA);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

void DebugLogger::logMicrophonef(const char* format, ...) {
    if (!(debug_categories & DEBUG_MICROPHONE)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_MICROPHONE);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

void DebugLogger::logBatteryf(const char* format, ...) {
    if (!(debug_categories & DEBUG_BATTERY)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_BATTERY);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

void DebugLogger::logPowerf(const char* format, ...) {
    if (!(debug_categories & DEBUG_POWER)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_POWER);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

void DebugLogger::logMemoryf(const char* format, ...) {
    if (!(debug_categories & DEBUG_MEMORY)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_MEMORY);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

void DebugLogger::logTimingf(const char* format, ...) {
    if (!(debug_categories & DEBUG_TIMING)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_SYSTEM);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

// ===================================================================
// TIMING AND PERFORMANCE MONITORING
// ===================================================================

unsigned long DebugLogger::startTiming(const char* operation) {
    if (!timing_enabled) return millis();
    
    unsigned long start_time = millis();
    
    // Find or create timing entry
    int index = findTimingEntry(operation);
    if (index == -1 && timing_entry_count < MAX_TIMING_ENTRIES) {
        index = timing_entry_count++;
        timing_entries[index].operation = operation;
    }
    
    if (index != -1) {
        timing_entries[index].start_time = start_time;
        timing_entries[index].active = true;
    }
    
    return start_time;
}

void DebugLogger::endTiming(unsigned long start_time, const char* operation) {
    if (!timing_enabled) return;
    
    unsigned long end_time = millis();
    unsigned long duration = end_time - start_time;
    
    // Update timing entry
    int index = findTimingEntry(operation);
    if (index != -1) {
        timing_entries[index].duration = duration;
        timing_entries[index].active = false;
    }
    
    logTimingResult(operation, duration);
    total_timing_operations++;
}

void DebugLogger::logTimingResult(const char* operation, unsigned long duration) {
    if (timing_enabled && (debug_categories & DEBUG_TIMING)) {
        SerialManager::debugf(MODULE_SYSTEM, "⏱️ %s: %lu ms", operation, duration);
    }
}

void DebugLogger::measureOperation(const char* operation, void (*func)()) {
    if (!func) return;
    
    unsigned long start = startTiming(operation);
    func();
    endTiming(start, operation);
}

void DebugLogger::measureOperationWithResult(const char* operation, bool (*func)(), bool& result) {
    if (!func) return;
    
    unsigned long start = startTiming(operation);
    result = func();
    endTiming(start, operation);
}

// ===================================================================
// MEMORY TRACKING
// ===================================================================

void DebugLogger::logMemoryUsage(const char* context) {
    if (!memory_tracking_enabled) return;
    
    size_t free_heap = ESP.getFreeHeap();
    size_t free_psram = ESP.getFreePsram();
    size_t largest_free_block = ESP.getMaxAllocHeap();
    
    if (context) {
        SerialManager::debugf(MODULE_MEMORY, "Memory [%s]: Heap=%zu KB, PSRAM=%zu KB, Largest=%zu KB",
                             context, free_heap / 1024, free_psram / 1024, largest_free_block / 1024);
    } else {
        SerialManager::debugf(MODULE_MEMORY, "Memory: Heap=%zu KB, PSRAM=%zu KB, Largest=%zu KB",
                             free_heap / 1024, free_psram / 1024, largest_free_block / 1024);
    }
}

void DebugLogger::logMemoryLeak(const char* operation, size_t bytes_before, size_t bytes_after) {
    if (!memory_tracking_enabled) return;
    
    if (bytes_after < bytes_before) {
        size_t leaked = bytes_before - bytes_after;
        SerialManager::warningf(MODULE_MEMORY, "⚠️ Memory leak detected in %s: %zu bytes", operation, leaked);
    }
}

void DebugLogger::logMemoryFragmentation() {
    if (!memory_tracking_enabled) return;
    
    size_t free_heap = ESP.getFreeHeap();
    size_t largest_free_block = ESP.getMaxAllocHeap();
    
    if (free_heap > 0) {
        float fragmentation = (1.0 - (float)largest_free_block / (float)free_heap) * 100.0;
        SerialManager::debugf(MODULE_MEMORY, "Memory fragmentation: %.1f%%", fragmentation);
    }
}

void DebugLogger::trackMemoryAllocation(const char* operation, size_t bytes) {
    if (memory_tracking_enabled && (debug_categories & DEBUG_MEMORY)) {
        SerialManager::debugf(MODULE_MEMORY, "Allocated %zu bytes for %s", bytes, operation);
    }
}

void DebugLogger::trackMemoryDeallocation(const char* operation, size_t bytes) {
    if (memory_tracking_enabled && (debug_categories & DEBUG_MEMORY)) {
        SerialManager::debugf(MODULE_MEMORY, "Deallocated %zu bytes from %s", bytes, operation);
    }
}

// ===================================================================
// SYSTEM INFORMATION DUMPS
// ===================================================================

void DebugLogger::dumpSystemInfo() {
    SerialManager::printHeader("System Information");
    
    SerialManager::infof(MODULE_SYSTEM, "Chip Model: %s", ESP.getChipModel());
    SerialManager::infof(MODULE_SYSTEM, "Chip Revision: %d", ESP.getChipRevision());
    SerialManager::infof(MODULE_SYSTEM, "CPU Frequency: %lu MHz", ESP.getCpuFreqMHz());
    SerialManager::infof(MODULE_SYSTEM, "Flash Size: %lu KB", ESP.getFlashChipSize() / 1024);
    SerialManager::infof(MODULE_SYSTEM, "PSRAM Size: %lu KB", ESP.getPsramSize() / 1024);
    SerialManager::infof(MODULE_SYSTEM, "Uptime: %lu ms", millis());
    
    SerialManager::printSeparator();
}

void DebugLogger::dumpMemoryInfo() {
    SerialManager::printHeader("Memory Information");
    
    size_t free_heap = ESP.getFreeHeap();
    size_t total_heap = ESP.getHeapSize();
    size_t free_psram = ESP.getFreePsram();
    size_t total_psram = ESP.getPsramSize();
    size_t largest_free_block = ESP.getMaxAllocHeap();
    
    SerialManager::infof(MODULE_MEMORY, "Heap: %zu / %zu KB (%.1f%% used)",
                        free_heap / 1024, total_heap / 1024,
                        (float)(total_heap - free_heap) / total_heap * 100.0);
    
    SerialManager::infof(MODULE_MEMORY, "PSRAM: %zu / %zu KB (%.1f%% used)",
                        free_psram / 1024, total_psram / 1024,
                        (float)(total_psram - free_psram) / total_psram * 100.0);
    
    SerialManager::infof(MODULE_MEMORY, "Largest free block: %zu KB", largest_free_block / 1024);
    
    if (free_heap > 0) {
        float fragmentation = (1.0 - (float)largest_free_block / (float)free_heap) * 100.0;
        SerialManager::infof(MODULE_MEMORY, "Heap fragmentation: %.1f%%", fragmentation);
    }
    
    SerialManager::printSeparator();
}

void DebugLogger::dumpTimingInfo() {
    if (!timing_enabled) return;
    
    SerialManager::printHeader("Timing Information");
    
    SerialManager::infof(MODULE_SYSTEM, "Total timing operations: %lu", total_timing_operations);
    SerialManager::infof(MODULE_SYSTEM, "Active timing entries: %d", timing_entry_count);
    
    for (int i = 0; i < timing_entry_count; i++) {
        if (timing_entries[i].operation) {
            SerialManager::infof(MODULE_SYSTEM, "  %s: %lu ms %s",
                               timing_entries[i].operation,
                               timing_entries[i].duration,
                               timing_entries[i].active ? "(active)" : "");
        }
    }
    
    SerialManager::printSeparator();
}

void DebugLogger::dumpPerformanceInfo() {
    if (!performance_monitoring_enabled) return;
    
    SerialManager::printHeader("Performance Information");
    
    SerialManager::infof(MODULE_SYSTEM, "Debug messages sent: %lu", total_debug_messages);
    SerialManager::infof(MODULE_SYSTEM, "Debug bytes sent: %lu", total_debug_bytes);
    
    unsigned long uptime = millis();
    if (uptime > 0) {
        SerialManager::infof(MODULE_SYSTEM, "Messages per second: %.2f", 
                           (float)total_debug_messages / (uptime / 1000.0));
        SerialManager::infof(MODULE_SYSTEM, "Bytes per second: %.2f", 
                           (float)total_debug_bytes / (uptime / 1000.0));
    }
    
    SerialManager::printSeparator();
}

void DebugLogger::dumpAllInfo() {
    dumpSystemInfo();
    dumpMemoryInfo();
    dumpTimingInfo();
    dumpPerformanceInfo();
}

// ===================================================================
// CONDITIONAL DEBUG METHODS
// ===================================================================

bool DebugLogger::isEnabled(uint8_t category) {
    return (debug_categories & category) != 0;
}

void DebugLogger::conditionalLog(uint8_t category, const char* message) {
    if (debug_categories & category) {
        SerialManager::debug(message, MODULE_SYSTEM);
        total_debug_messages++;
        total_debug_bytes += strlen(message);
    }
}

void DebugLogger::conditionalLogf(uint8_t category, const char* format, ...) {
    if (!(debug_categories & category)) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    SerialManager::debug(buffer, MODULE_SYSTEM);
    
    va_end(args);
    
    total_debug_messages++;
    total_debug_bytes += strlen(buffer);
}

// ===================================================================
// ADVANCED DEBUGGING FEATURES
// ===================================================================

void DebugLogger::logStackUsage() {
    // ESP32-specific stack usage logging
    SerialManager::debugf(MODULE_SYSTEM, "Stack high water mark: %lu bytes", 
                         uxTaskGetStackHighWaterMark(NULL));
}

void DebugLogger::logTaskInfo() {
    // Basic task information
    SerialManager::debugf(MODULE_SYSTEM, "Current task: %s", pcTaskGetTaskName(NULL));
}

void DebugLogger::logInterruptInfo() {
    // Interrupt-related debugging would go here
    SerialManager::debug("Interrupt info not implemented", MODULE_SYSTEM);
}

void DebugLogger::logHeapFragmentation() {
    logMemoryFragmentation();
}

// ===================================================================
// PERIODIC MONITORING
// ===================================================================

void DebugLogger::update() {
    periodicMemoryCheck();
    periodicPerformanceReport();
}

void DebugLogger::periodicMemoryCheck() {
    if (!memory_tracking_enabled) return;
    
    unsigned long current_time = millis();
    if (current_time - last_memory_sample >= MEMORY_SAMPLE_INTERVAL_MS) {
        addMemorySample();
        last_memory_sample = current_time;
    }
}

void DebugLogger::periodicPerformanceReport() {
    if (!performance_monitoring_enabled) return;
    
    unsigned long current_time = millis();
    if (current_time - last_performance_report >= PERFORMANCE_REPORT_INTERVAL_MS) {
        reportPerformanceMetrics();
        last_performance_report = current_time;
    }
}

// ===================================================================
// UTILITY METHODS
// ===================================================================

void DebugLogger::hexDump(const char* label, const uint8_t* data, size_t length) {
    if (!(debug_categories & DEBUG_MEMORY)) return;
    
    SerialManager::debugf(MODULE_SYSTEM, "%s hex dump (%zu bytes):", label, length);
    
    for (size_t i = 0; i < length; i += 16) {
        SerialManager::printf("  %04X: ", i);
        
        // Print hex values
        for (size_t j = 0; j < 16 && (i + j) < length; j++) {
            SerialManager::printf("%02X ", data[i + j]);
        }
        
        // Print ASCII values
        SerialManager::print(" |");
        for (size_t j = 0; j < 16 && (i + j) < length; j++) {
            char c = data[i + j];
            SerialManager::printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        SerialManager::println("|");
    }
}

void DebugLogger::binaryDump(const char* label, const uint8_t* data, size_t length) {
    if (!(debug_categories & DEBUG_MEMORY)) return;
    
    SerialManager::debugf(MODULE_SYSTEM, "%s binary dump (%zu bytes):", label, length);
    
    for (size_t i = 0; i < length && i < 32; i++) {
        SerialManager::printf("%08b ", data[i]);
        if ((i + 1) % 8 == 0) {
            SerialManager::println("");
        }
    }
    
    if (length > 32) {
        SerialManager::println("... (truncated)");
    }
}

void DebugLogger::asciiDump(const char* label, const uint8_t* data, size_t length) {
    if (!(debug_categories & DEBUG_MEMORY)) return;
    
    SerialManager::debugf(MODULE_SYSTEM, "%s ASCII dump (%zu bytes):", label, length);
    SerialManager::print("  ");
    
    for (size_t i = 0; i < length && i < 128; i++) {
        char c = data[i];
        SerialManager::printf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    
    SerialManager::println("");
    
    if (length > 128) {
        SerialManager::println("... (truncated)");
    }
}

// ===================================================================
// PRIVATE HELPER METHODS
// ===================================================================

int DebugLogger::findTimingEntry(const char* operation) {
    for (int i = 0; i < timing_entry_count; i++) {
        if (timing_entries[i].operation && strcmp(timing_entries[i].operation, operation) == 0) {
            return i;
        }
    }
    return -1;
}

void DebugLogger::addMemorySample() {
    if (memory_sample_count >= 10) {
        // Shift samples down
        for (int i = 0; i < 9; i++) {
            memory_samples[i] = memory_samples[i + 1];
        }
        memory_sample_count = 9;
    }
    
    MemorySample& sample = memory_samples[memory_sample_count++];
    sample.timestamp = millis();
    sample.free_heap = ESP.getFreeHeap();
    sample.free_psram = ESP.getFreePsram();
    sample.largest_free_block = ESP.getMaxAllocHeap();
}

void DebugLogger::reportPerformanceMetrics() {
    SerialManager::infof(MODULE_SYSTEM, "Debug performance: %lu messages, %lu bytes in last period",
                        total_debug_messages, total_debug_bytes);
} 