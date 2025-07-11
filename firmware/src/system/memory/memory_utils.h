#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include "../../hal/xiao_esp32s3_constants.h"

// ===================================================================
// MEMORY MANAGEMENT UTILITIES FOR ESP32-S3
// ===================================================================
//
// Compatible with ESP32 Arduino Core 2.0.17 and existing ps_calloc usage
// Provides memory monitoring, leak detection, and safe allocation
//

/**
 * Memory allocation preferences
 */
typedef enum {
    MEM_PREFER_PSRAM,      // Prefer PSRAM, fallback to DRAM
    MEM_PREFER_DRAM,       // Prefer DRAM, fallback to PSRAM
    MEM_FORCE_PSRAM,       // Force PSRAM only
    MEM_FORCE_DRAM,        // Force DRAM only
    MEM_AUTO               // Automatic selection based on size
} memory_preference_t;

/**
 * Memory allocation tracking
 */
typedef struct {
    void* ptr;
    size_t size;
    uint32_t caps;
    unsigned long timestamp;
    const char* tag;
    bool active;
} memory_allocation_t;

/**
 * Memory statistics
 */
typedef struct {
    // PSRAM statistics
    size_t psram_total;
    size_t psram_free;
    size_t psram_used;
    size_t psram_largest_free;
    
    // DRAM statistics
    size_t dram_total;
    size_t dram_free;
    size_t dram_used;
    size_t dram_largest_free;
    
    // Overall statistics
    size_t total_allocations;
    size_t active_allocations;
    size_t peak_allocations;
    size_t total_allocated_bytes;
    size_t peak_allocated_bytes;
    
    // Fragmentation
    float psram_fragmentation;
    float dram_fragmentation;
    
    // Health indicators
    bool psram_available;
    bool memory_pressure;
    bool fragmentation_warning;
    
    unsigned long last_update;
} memory_stats_t;

// Global memory statistics
extern memory_stats_t memoryStats;

// Memory allocation tracking (limited to prevent overhead)
#define MAX_TRACKED_ALLOCATIONS 32
extern memory_allocation_t trackedAllocations[MAX_TRACKED_ALLOCATIONS];

// ===================================================================
// FUNCTION DECLARATIONS
// ===================================================================

/**
 * Initialize memory management system
 */
void initializeMemoryManager();

/**
 * Update memory statistics
 */
void updateMemoryStats();

/**
 * Track memory allocation
 */
void trackAllocation(void* ptr, size_t size, uint32_t caps, const char* tag);

/**
 * Untrack memory allocation
 */
void untrackAllocation(void* ptr);

/**
 * Safe memory allocation with preference and tracking
 */
void* safeAllocate(size_t size, memory_preference_t preference, const char* tag = "unknown");

/**
 * Safe memory deallocation with tracking
 */
void safeFree(void* ptr);

/**
 * PS_CALLOC wrapper with tracking
 */
void* ps_calloc_tracked(size_t count, size_t size, const char* tag = "ps_calloc");

/**
 * Check if allocation is safe
 */
bool isAllocationSafe(size_t size, memory_preference_t preference);

/**
 * Get memory usage percentage
 */
float getMemoryUsagePercent(bool psram = false);

/**
 * Check for memory leaks
 */
int checkMemoryLeaks();

/**
 * Print memory statistics
 */
void printMemoryStats();

/**
 * Print tracked allocations
 */
void printTrackedAllocations();

/**
 * Emergency memory cleanup
 */
void emergencyMemoryCleanup();

/**
 * Memory health check
 */
bool memoryHealthCheck();

// ===================================================================
// CONVENIENCE MACROS
// ===================================================================

// Tracked allocation macros for backward compatibility
#define PS_CALLOC_TRACKED(count, size, tag) ps_calloc_tracked(count, size, tag)
#define SAFE_ALLOCATE(size, pref, tag) safeAllocate(size, pref, tag)
#define SAFE_FREE(ptr) safeFree(ptr)

// ===================================================================
// MEMORY MONITORING CONSTANTS
// ===================================================================

// Memory monitoring intervals
#define MEMORY_UPDATE_INTERVAL 5000      // 5 seconds
#define MEMORY_LEAK_CHECK_INTERVAL 300000 // 5 minutes

// Memory thresholds
#define MEMORY_CRITICAL_DRAM_KB 16       // 16KB
#define MEMORY_CRITICAL_PSRAM_KB 32      // 32KB
#define MEMORY_FRAGMENTATION_THRESHOLD 0.7f

#endif // MEMORY_UTILS_H 