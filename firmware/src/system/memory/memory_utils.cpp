#include "memory_utils.h"

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

// All functions are implemented as static inline in the header file
// for performance reasons and to maintain compatibility with existing
// ESP32 Arduino Core 2.0.17 patterns.

// This implementation file only contains the global variable definitions
// to avoid multiple definition errors during linking. 