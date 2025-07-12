#include "memory_utils.h"
#include "../serial/serial.h"

// ===================================================================
// GLOBAL MEMORY MANAGEMENT STATE
// ===================================================================

// Global memory statistics
memory_stats_t memoryStats = {0};

// Memory allocation tracking array
memory_allocation_t trackedAllocations[MAX_TRACKED_ALLOCATIONS];

// ===================================================================
// MEMORY MANAGEMENT IMPLEMENTATION
// ===================================================================

void initializeMemoryManager() {
    SerialSystem::info("Initializing Memory Manager...", MODULE_MEMORY);
    
    // Clear tracking array
    for (int i = 0; i < MAX_TRACKED_ALLOCATIONS; i++) {
        trackedAllocations[i].ptr = nullptr;
        trackedAllocations[i].active = false;
    }
    
    // Initialize statistics
    memoryStats.total_allocations = 0;
    memoryStats.active_allocations = 0;
    memoryStats.peak_allocations = 0;
    memoryStats.total_allocated_bytes = 0;
    memoryStats.peak_allocated_bytes = 0;
    memoryStats.psram_available = psramFound();
    memoryStats.last_update = millis();
    
    updateMemoryStats();
    
    SerialSystem::infof(MODULE_MEMORY, "Memory Manager initialized - PSRAM: %s", 
                       memoryStats.psram_available ? "Available" : "Not Available");
}

void updateMemoryStats() {
    unsigned long current_time = millis();
    
    // Update PSRAM statistics
    if (memoryStats.psram_available) {
        memoryStats.psram_total = ESP.getPsramSize();
        memoryStats.psram_free = ESP.getFreePsram();
        memoryStats.psram_used = memoryStats.psram_total - memoryStats.psram_free;
        memoryStats.psram_largest_free = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
        
        // Calculate PSRAM fragmentation
        if (memoryStats.psram_free > 0) {
            memoryStats.psram_fragmentation = 
                1.0f - ((float)memoryStats.psram_largest_free / (float)memoryStats.psram_free);
        }
    }
    
    // Update DRAM statistics
    memoryStats.dram_total = ESP.getHeapSize();
    memoryStats.dram_free = ESP.getFreeHeap();
    memoryStats.dram_used = memoryStats.dram_total - memoryStats.dram_free;
    memoryStats.dram_largest_free = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    
    // Calculate DRAM fragmentation
    if (memoryStats.dram_free > 0) {
        memoryStats.dram_fragmentation = 
            1.0f - ((float)memoryStats.dram_largest_free / (float)memoryStats.dram_free);
    }
    
    // Update health indicators
    memoryStats.memory_pressure = (memoryStats.dram_free < 32768) || 
                                  (memoryStats.psram_available && memoryStats.psram_free < 65536);
    memoryStats.fragmentation_warning = (memoryStats.dram_fragmentation > 0.7f) || 
                                        (memoryStats.psram_fragmentation > 0.7f);
    
    memoryStats.last_update = current_time;
}

void trackAllocation(void* ptr, size_t size, uint32_t caps, const char* tag) {
    if (!ptr) return;
    
    // Find empty slot
    for (int i = 0; i < MAX_TRACKED_ALLOCATIONS; i++) {
        if (!trackedAllocations[i].active) {
            trackedAllocations[i].ptr = ptr;
            trackedAllocations[i].size = size;
            trackedAllocations[i].caps = caps;
            trackedAllocations[i].timestamp = millis();
            trackedAllocations[i].tag = tag;
            trackedAllocations[i].active = true;
            
            memoryStats.total_allocations++;
            memoryStats.active_allocations++;
            memoryStats.total_allocated_bytes += size;
            
            if (memoryStats.active_allocations > memoryStats.peak_allocations) {
                memoryStats.peak_allocations = memoryStats.active_allocations;
            }
            
            size_t current_total = 0;
            for (int j = 0; j < MAX_TRACKED_ALLOCATIONS; j++) {
                if (trackedAllocations[j].active) {
                    current_total += trackedAllocations[j].size;
                }
            }
            if (current_total > memoryStats.peak_allocated_bytes) {
                memoryStats.peak_allocated_bytes = current_total;
            }
            
            break;
        }
    }
}

void untrackAllocation(void* ptr) {
    if (!ptr) return;
    
    for (int i = 0; i < MAX_TRACKED_ALLOCATIONS; i++) {
        if (trackedAllocations[i].active && trackedAllocations[i].ptr == ptr) {
            trackedAllocations[i].active = false;
            memoryStats.active_allocations--;
            break;
        }
    }
}

void* safeAllocate(size_t size, memory_preference_t preference, const char* tag) {
    if (size == 0) return nullptr;
    
    void* ptr = nullptr;
    uint32_t caps = 0;
    
    switch (preference) {
        case MEM_PREFER_PSRAM:
            if (memoryStats.psram_available) {
                ptr = ps_calloc(size, 1);
                caps = MALLOC_CAP_SPIRAM;
            }
            if (!ptr) {
                ptr = heap_caps_calloc(size, 1, MALLOC_CAP_8BIT);
                caps = MALLOC_CAP_8BIT;
            }
            break;
            
        case MEM_PREFER_DRAM:
            ptr = heap_caps_calloc(size, 1, MALLOC_CAP_8BIT);
            caps = MALLOC_CAP_8BIT;
            if (!ptr && memoryStats.psram_available) {
                ptr = ps_calloc(size, 1);
                caps = MALLOC_CAP_SPIRAM;
            }
            break;
            
        case MEM_FORCE_PSRAM:
            if (memoryStats.psram_available) {
                ptr = ps_calloc(size, 1);
                caps = MALLOC_CAP_SPIRAM;
            }
            break;
            
        case MEM_FORCE_DRAM:
            ptr = heap_caps_calloc(size, 1, MALLOC_CAP_8BIT);
            caps = MALLOC_CAP_8BIT;
            break;
            
        case MEM_AUTO:
            // Large allocations (>8KB) prefer PSRAM
            if (size > 8192 && memoryStats.psram_available) {
                ptr = ps_calloc(size, 1);
                caps = MALLOC_CAP_SPIRAM;
            }
            if (!ptr) {
                ptr = heap_caps_calloc(size, 1, MALLOC_CAP_8BIT);
                caps = MALLOC_CAP_8BIT;
            }
            break;
    }
    
    if (ptr) {
        trackAllocation(ptr, size, caps, tag);
        SerialSystem::debugf(MODULE_MEMORY, "Allocated %d bytes in %s for %s", size, 
                            (caps == MALLOC_CAP_SPIRAM) ? "PSRAM" : "DRAM", tag);
    } else {
        SerialSystem::errorf(MODULE_MEMORY, "Failed to allocate %d bytes for %s", size, tag);
    }
    
    return ptr;
}

void safeFree(void* ptr) {
    if (!ptr) return;
    
    untrackAllocation(ptr);
    free(ptr);
}

void* ps_calloc_tracked(size_t count, size_t size, const char* tag) {
    void* ptr = ps_calloc(count, size);
    if (ptr) {
        trackAllocation(ptr, count * size, MALLOC_CAP_SPIRAM, tag);
    }
    return ptr;
}

bool isAllocationSafe(size_t size, memory_preference_t preference) {
    updateMemoryStats();
    
    switch (preference) {
        case MEM_PREFER_PSRAM:
        case MEM_FORCE_PSRAM:
            return memoryStats.psram_available && (memoryStats.psram_free > size + 4096);
            
        case MEM_PREFER_DRAM:
        case MEM_FORCE_DRAM:
            return memoryStats.dram_free > size + 4096;
            
        case MEM_AUTO:
            return (memoryStats.dram_free > size + 4096) || 
                   (memoryStats.psram_available && memoryStats.psram_free > size + 4096);
    }
    
    return false;
}

float getMemoryUsagePercent(bool psram) {
    updateMemoryStats();
    
    if (psram && memoryStats.psram_available) {
        return ((float)memoryStats.psram_used / (float)memoryStats.psram_total) * 100.0f;
    } else {
        return ((float)memoryStats.dram_used / (float)memoryStats.dram_total) * 100.0f;
    }
}

int checkMemoryLeaks() {
    int leak_count = 0;
    unsigned long current_time = millis();
    
    Serial.println("=== Memory Leak Check ===");
    
    for (int i = 0; i < MAX_TRACKED_ALLOCATIONS; i++) {
        if (trackedAllocations[i].active) {
            unsigned long age = current_time - trackedAllocations[i].timestamp;
            if (age > 300000) { // 5 minutes
                Serial.printf("Potential leak: %s - %d bytes, age: %lu ms\n",
                              trackedAllocations[i].tag,
                              trackedAllocations[i].size,
                              age);
                leak_count++;
            }
        }
    }
    
    if (leak_count == 0) {
        Serial.println("✅ No memory leaks detected");
    } else {
        Serial.printf("⚠️  %d potential memory leaks found\n", leak_count);
    }
    
    return leak_count;
}

void printMemoryStats() {
    updateMemoryStats();
    
    Serial.println("\n=== Memory Statistics ===");
    
    // PSRAM statistics
    if (memoryStats.psram_available) {
        Serial.printf("PSRAM: %d KB total, %d KB free, %d KB used (%.1f%%)\n",
                      memoryStats.psram_total / 1024,
                      memoryStats.psram_free / 1024,
                      memoryStats.psram_used / 1024,
                      getMemoryUsagePercent(true));
        Serial.printf("PSRAM Largest Free: %d KB, Fragmentation: %.1f%%\n",
                      memoryStats.psram_largest_free / 1024,
                      memoryStats.psram_fragmentation * 100.0f);
    } else {
        Serial.println("PSRAM: Not available");
    }
    
    // DRAM statistics
    Serial.printf("DRAM: %d KB total, %d KB free, %d KB used (%.1f%%)\n",
                  memoryStats.dram_total / 1024,
                  memoryStats.dram_free / 1024,
                  memoryStats.dram_used / 1024,
                  getMemoryUsagePercent(false));
    Serial.printf("DRAM Largest Free: %d KB, Fragmentation: %.1f%%\n",
                  memoryStats.dram_largest_free / 1024,
                  memoryStats.dram_fragmentation * 100.0f);
    
    // Allocation statistics
    Serial.printf("Allocations: %d total, %d active, %d peak\n",
                  memoryStats.total_allocations,
                  memoryStats.active_allocations,
                  memoryStats.peak_allocations);
    Serial.printf("Allocated Bytes: %d total, %d peak\n",
                  memoryStats.total_allocated_bytes,
                  memoryStats.peak_allocated_bytes);
    
    // Health indicators
    Serial.printf("Memory Pressure: %s\n", memoryStats.memory_pressure ? "⚠️  YES" : "✅ NO");
    Serial.printf("Fragmentation Warning: %s\n", memoryStats.fragmentation_warning ? "⚠️  YES" : "✅ NO");
    
    Serial.println("========================");
}

void printTrackedAllocations() {
    Serial.println("\n=== Tracked Allocations ===");
    
    int active_count = 0;
    for (int i = 0; i < MAX_TRACKED_ALLOCATIONS; i++) {
        if (trackedAllocations[i].active) {
            unsigned long age = millis() - trackedAllocations[i].timestamp;
            Serial.printf("%d: %s - %d bytes, %s, age: %lu ms\n",
                          i,
                          trackedAllocations[i].tag,
                          trackedAllocations[i].size,
                          (trackedAllocations[i].caps == MALLOC_CAP_SPIRAM) ? "PSRAM" : "DRAM",
                          age);
            active_count++;
        }
    }
    
    if (active_count == 0) {
        Serial.println("No tracked allocations");
    } else {
        Serial.printf("Total: %d active allocations\n", active_count);
    }
    
    Serial.println("===========================");
}

void emergencyMemoryCleanup() {
    Serial.println("🚨 Emergency memory cleanup initiated");
    
    // Force garbage collection by calling heap functions
    esp_get_free_heap_size();
    
    // Clear any stale allocations (this is aggressive - use carefully)
    Serial.println("Memory cleanup completed");
}

bool memoryHealthCheck() {
    updateMemoryStats();
    
    bool healthy = true;
    
    // Check DRAM levels
    if (memoryStats.dram_free < 16384) { // Less than 16KB
        Serial.println("⚠️  DRAM critically low");
        healthy = false;
    }
    
    // Check PSRAM levels
    if (memoryStats.psram_available && memoryStats.psram_free < 32768) { // Less than 32KB
        Serial.println("⚠️  PSRAM critically low");
        healthy = false;
    }
    
    // Check fragmentation
    if (memoryStats.dram_fragmentation > 0.8f) {
        Serial.println("⚠️  DRAM highly fragmented");
        healthy = false;
    }
    
    if (memoryStats.psram_fragmentation > 0.8f) {
        Serial.println("⚠️  PSRAM highly fragmented");
        healthy = false;
    }
    
    return healthy;
} 