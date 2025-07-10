#ifndef CYCLE_MANAGER_H
#define CYCLE_MANAGER_H

#include <Arduino.h>
#include <functional>

// ===================================================================
// CENTRALIZED CYCLE MANAGEMENT SYSTEM
// ===================================================================

/**
 * Cycle execution modes
 */
typedef enum {
    CYCLE_MODE_INTERVAL,        // Execute at regular intervals
    CYCLE_MODE_TIMEOUT,         // Execute after timeout
    CYCLE_MODE_CONDITION,       // Execute when condition is met
    CYCLE_MODE_PATTERN,         // Execute following a pattern
    CYCLE_MODE_CIRCULAR_BUFFER, // Manage circular buffer updates
    CYCLE_MODE_STATE_MACHINE    // State machine transitions
} cycle_mode_t;

/**
 * Cycle priority levels
 */
typedef enum {
    CYCLE_PRIORITY_CRITICAL = 0,    // Always execute (LED updates, safety checks)
    CYCLE_PRIORITY_HIGH = 1,        // High priority (BLE, audio)
    CYCLE_PRIORITY_NORMAL = 2,      // Normal priority (photos, video)
    CYCLE_PRIORITY_LOW = 3,         // Low priority (statistics, cleanup)
    CYCLE_PRIORITY_BACKGROUND = 4   // Background tasks (history, logging)
} cycle_priority_t;

/**
 * Cycle state
 */
typedef enum {
    CYCLE_STATE_INACTIVE,       // Cycle is not running
    CYCLE_STATE_ACTIVE,         // Cycle is running normally
    CYCLE_STATE_PAUSED,         // Cycle is temporarily paused
    CYCLE_STATE_ERROR,          // Cycle encountered an error
    CYCLE_STATE_COMPLETED       // One-shot cycle completed
} cycle_state_t;

/**
 * Pattern step definition for pattern cycles
 */
typedef struct {
    uint32_t duration_ms;       // Duration of this step
    uint8_t value;             // Value for this step (0-255)
    bool active;               // Whether this step is active
} pattern_step_t;

/**
 * Circular buffer configuration
 */
typedef struct {
    void* buffer;              // Pointer to buffer
    size_t element_size;       // Size of each element
    size_t buffer_size;        // Total number of elements
    size_t* index;             // Pointer to current index
    std::function<void(void*)> on_wrap; // Callback when buffer wraps
} circular_buffer_config_t;

/**
 * Cycle configuration structure
 */
typedef struct {
    const char* name;                           // Cycle name for debugging
    cycle_mode_t mode;                          // Execution mode
    cycle_priority_t priority;                  // Priority level
    uint32_t interval_ms;                       // Interval for INTERVAL mode
    uint32_t timeout_ms;                        // Timeout for TIMEOUT mode
    std::function<bool()> condition;            // Condition function for CONDITION mode
    std::function<void()> execute;              // Execution function
    std::function<void()> on_error;             // Error handler
    pattern_step_t* pattern;                    // Pattern steps for PATTERN mode
    size_t pattern_length;                      // Number of pattern steps
    circular_buffer_config_t* buffer_config;    // Buffer config for CIRCULAR_BUFFER mode
    bool enabled;                               // Whether cycle is enabled
    bool one_shot;                              // Execute only once
} cycle_config_t;

/**
 * Cycle runtime state
 */
typedef struct {
    cycle_state_t state;                        // Current state
    unsigned long last_execution;               // Last execution time
    unsigned long next_execution;               // Next scheduled execution
    size_t execution_count;                     // Number of executions
    size_t error_count;                         // Number of errors
    unsigned long total_execution_time;         // Total time spent executing
    unsigned long max_execution_time;           // Maximum single execution time
    size_t pattern_step;                        // Current pattern step
    bool pattern_step_active;                   // Whether current pattern step is active
    unsigned long pattern_step_start;           // When current pattern step started
} cycle_runtime_t;

/**
 * Main cycle structure
 */
typedef struct {
    cycle_config_t config;                      // Configuration
    cycle_runtime_t runtime;                    // Runtime state
} cycle_t;

// Maximum number of cycles that can be managed
#define MAX_CYCLES 32

// Global cycle manager state
extern cycle_t cycles[MAX_CYCLES];
extern size_t cycle_count;
extern bool cycle_manager_initialized;

// Statistics
extern unsigned long total_cycles_executed;
extern unsigned long total_execution_time;
extern unsigned long last_manager_update;

/**
 * Initialize the cycle manager
 */
void initializeCycleManager();

/**
 * Register a new cycle
 * @param config Cycle configuration
 * @return Cycle ID (index) or -1 if failed
 */
int registerCycle(const cycle_config_t& config);

/**
 * Update all cycles (call this in main loop)
 */
void updateCycles();

/**
 * Enable/disable a cycle
 * @param cycle_id Cycle ID
 * @param enabled Whether to enable the cycle
 */
void setCycleEnabled(int cycle_id, bool enabled);

/**
 * Pause/resume a cycle
 * @param cycle_id Cycle ID
 * @param paused Whether to pause the cycle
 */
void setCyclePaused(int cycle_id, bool paused);

/**
 * Get cycle state
 * @param cycle_id Cycle ID
 * @return Current cycle state
 */
cycle_state_t getCycleState(int cycle_id);

/**
 * Get cycle statistics
 * @param cycle_id Cycle ID
 * @return Pointer to runtime statistics
 */
const cycle_runtime_t* getCycleStats(int cycle_id);

/**
 * Reset cycle statistics
 * @param cycle_id Cycle ID
 */
void resetCycleStats(int cycle_id);

/**
 * Print cycle manager statistics
 */
void printCycleManagerStats();

/**
 * Print individual cycle statistics
 * @param cycle_id Cycle ID
 */
void printCycleStats(int cycle_id);

/**
 * Internal functions (used by cycle manager)
 */
void executeCycle(cycle_t* cycle, unsigned long current_time);
bool updatePatternCycle(cycle_t* cycle, unsigned long current_time);
const char* getCycleStateString(cycle_state_t state);
const char* getCycleModeString(cycle_mode_t mode);

// ===================================================================
// CONVENIENCE FUNCTIONS FOR COMMON CYCLE TYPES
// ===================================================================

/**
 * Register an interval-based cycle
 * @param name Cycle name
 * @param interval_ms Execution interval
 * @param execute Execution function
 * @param priority Priority level
 * @return Cycle ID or -1 if failed
 */
int registerIntervalCycle(const char* name, uint32_t interval_ms, 
                         std::function<void()> execute, 
                         cycle_priority_t priority = CYCLE_PRIORITY_NORMAL);

/**
 * Register a timeout-based cycle
 * @param name Cycle name
 * @param timeout_ms Timeout duration
 * @param execute Execution function
 * @param priority Priority level
 * @return Cycle ID or -1 if failed
 */
int registerTimeoutCycle(const char* name, uint32_t timeout_ms, 
                        std::function<void()> execute,
                        cycle_priority_t priority = CYCLE_PRIORITY_NORMAL);

/**
 * Register a condition-based cycle
 * @param name Cycle name
 * @param condition Condition function
 * @param execute Execution function
 * @param priority Priority level
 * @return Cycle ID or -1 if failed
 */
int registerConditionCycle(const char* name, std::function<bool()> condition,
                          std::function<void()> execute,
                          cycle_priority_t priority = CYCLE_PRIORITY_NORMAL);

/**
 * Register a pattern-based cycle (for LED patterns, etc.)
 * @param name Cycle name
 * @param pattern Pattern steps
 * @param pattern_length Number of pattern steps
 * @param execute Execution function (receives current step)
 * @param priority Priority level
 * @return Cycle ID or -1 if failed
 */
int registerPatternCycle(const char* name, pattern_step_t* pattern, size_t pattern_length,
                        std::function<void()> execute,
                        cycle_priority_t priority = CYCLE_PRIORITY_NORMAL);

/**
 * Register a circular buffer cycle
 * @param name Cycle name
 * @param buffer_config Buffer configuration
 * @param execute Execution function
 * @param priority Priority level
 * @return Cycle ID or -1 if failed
 */
int registerCircularBufferCycle(const char* name, circular_buffer_config_t* buffer_config,
                               std::function<void()> execute,
                               cycle_priority_t priority = CYCLE_PRIORITY_NORMAL);

// ===================================================================
// SPECIALIZED CYCLE MANAGERS
// ===================================================================

/**
 * Charging cycle manager
 */
namespace ChargingCycles {
    void initialize();
    void registerChargingStatusCycle();
    void registerChargingHistoryCycle();
    void registerChargingSafetyCycle();
}

/**
 * LED pattern cycle manager
 */
namespace LEDCycles {
    void initialize();
    void registerLEDUpdateCycle();
    void registerPatternCycles();
}

/**
 * Power management cycle manager
 */
namespace PowerCycles {
    void initialize();
    void registerBatteryUpdateCycle();
    void registerPowerStatsCycle();
    void registerSleepManagementCycle();
}

/**
 * Data acquisition cycle manager
 */
namespace DataCycles {
    void initialize();
    void registerAudioCaptureCycle();
    void registerPhotoCycle();
    void registerVideoStreamCycle();
}

/**
 * Communication cycle manager
 */
namespace CommCycles {
    void initialize();
    void registerBLEUpdateCycle();
    void registerDataTransmissionCycle();
}

#endif // CYCLE_MANAGER_H 