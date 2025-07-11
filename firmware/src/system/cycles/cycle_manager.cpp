#include "cycle_manager.h"
#include <Arduino.h>
#include "../clock/timing.h"

// ===================================================================
// GLOBAL CYCLE MANAGER STATE
// ===================================================================

cycle_t cycles[MAX_CYCLES];
size_t cycle_count = 0;
bool cycle_manager_initialized = false;

// Statistics
unsigned long total_cycles_executed = 0;
unsigned long total_execution_time = 0;
unsigned long last_manager_update = 0;

// ===================================================================
// CORE CYCLE MANAGER FUNCTIONS
// ===================================================================

void initializeCycleManager() {
    if (cycle_manager_initialized) {
        return;
    }
    
    Serial.println("Initializing Cycle Manager...");
    
    // Initialize all cycles
    for (size_t i = 0; i < MAX_CYCLES; i++) {
        cycles[i].config.name = nullptr;
        cycles[i].config.enabled = false;
        cycles[i].runtime.state = CYCLE_STATE_INACTIVE;
        cycles[i].runtime.last_execution = 0;
        cycles[i].runtime.next_execution = 0;
        cycles[i].runtime.execution_count = 0;
        cycles[i].runtime.error_count = 0;
        cycles[i].runtime.total_execution_time = 0;
        cycles[i].runtime.max_execution_time = 0;
        cycles[i].runtime.pattern_step = 0;
        cycles[i].runtime.pattern_step_active = false;
        cycles[i].runtime.pattern_step_start = 0;
    }
    
    cycle_count = 0;
    total_cycles_executed = 0;
    total_execution_time = 0;
    last_manager_update = measureStart();
    
    cycle_manager_initialized = true;
    Serial.println("Cycle Manager initialized");
}

int registerCycle(const cycle_config_t& config) {
    if (!cycle_manager_initialized) {
        Serial.println("Cycle manager not initialized!");
        return -1;
    }
    
    if (cycle_count >= MAX_CYCLES) {
        Serial.printf("Maximum cycles reached (%d)\n", MAX_CYCLES);
        return -1;
    }
    
    if (!config.name || !config.execute) {
        Serial.println("Invalid cycle configuration");
        return -1;
    }
    
    int cycle_id = cycle_count++;
    cycles[cycle_id].config = config;
    cycles[cycle_id].runtime.state = config.enabled ? CYCLE_STATE_ACTIVE : CYCLE_STATE_INACTIVE;
    cycles[cycle_id].runtime.last_execution = measureStart();
    cycles[cycle_id].runtime.next_execution = measureStart() + config.interval_ms;
    
    Serial.printf("Registered cycle '%s' (ID: %d, Priority: %d)\n", 
                  config.name, cycle_id, config.priority);
    
    return cycle_id;
}

void updateCycles() {
    if (!cycle_manager_initialized) {
        return;
    }
    
    unsigned long current_time = measureStart();
    unsigned long manager_start = current_time;
    
    // Process cycles by priority
    for (int priority = CYCLE_PRIORITY_CRITICAL; priority <= CYCLE_PRIORITY_BACKGROUND; priority++) {
        for (size_t i = 0; i < cycle_count; i++) {
            cycle_t* cycle = &cycles[i];
            
            // Skip if wrong priority or not active
            if (cycle->config.priority != priority || 
                cycle->runtime.state != CYCLE_STATE_ACTIVE ||
                !cycle->config.enabled) {
                continue;
            }
            
            bool should_execute = false;
            
            // Check execution condition based on mode
            switch (cycle->config.mode) {
                case CYCLE_MODE_INTERVAL:
                    should_execute = hasTimedOut(cycle->runtime.last_execution, cycle->config.interval_ms);
                    break;
                    
                case CYCLE_MODE_TIMEOUT:
                    should_execute = hasTimedOut(cycle->runtime.last_execution, cycle->config.timeout_ms);
                    if (should_execute && cycle->config.one_shot) {
                        cycle->runtime.state = CYCLE_STATE_COMPLETED;
                    }
                    break;
                    
                case CYCLE_MODE_CONDITION:
                    if (cycle->config.condition) {
                        should_execute = cycle->config.condition();
                    }
                    break;
                    
                case CYCLE_MODE_PATTERN:
                    should_execute = updatePatternCycle(cycle, current_time);
                    break;
                    
                case CYCLE_MODE_CIRCULAR_BUFFER:
                    should_execute = true; // Always execute for buffer management
                    break;
                    
                case CYCLE_MODE_STATE_MACHINE:
                    should_execute = true; // State machines manage their own timing
                    break;
            }
            
            if (should_execute) {
                executeCycle(cycle, current_time);
            }
        }
    }
    
    // Update manager statistics
    unsigned long manager_duration = measureEnd(manager_start);
    total_execution_time += manager_duration;
    last_manager_update = current_time;
}

void executeCycle(cycle_t* cycle, unsigned long current_time) {
    unsigned long execution_start = current_time;
    
    try {
        // Execute the cycle
        cycle->config.execute();
        
        // Update statistics
        cycle->runtime.execution_count++;
        cycle->runtime.last_execution = current_time;
        total_cycles_executed++;
        
        // Calculate execution time
        unsigned long execution_time = measureEnd(execution_start);
        cycle->runtime.total_execution_time += execution_time;
        if (execution_time > cycle->runtime.max_execution_time) {
            cycle->runtime.max_execution_time = execution_time;
        }
        
        // Handle one-shot cycles
        if (cycle->config.one_shot) {
            cycle->runtime.state = CYCLE_STATE_COMPLETED;
        }
        
    } catch (...) {
        // Handle execution error
        cycle->runtime.error_count++;
        cycle->runtime.state = CYCLE_STATE_ERROR;
        
        Serial.printf("Cycle '%s' execution error!\n", cycle->config.name);
        
        if (cycle->config.on_error) {
            cycle->config.on_error();
        }
    }
}

bool updatePatternCycle(cycle_t* cycle, unsigned long current_time) {
    if (!cycle->config.pattern || cycle->config.pattern_length == 0) {
        return false;
    }
    
    // Check if current pattern step is complete
    pattern_step_t* current_step = &cycle->config.pattern[cycle->runtime.pattern_step];
    unsigned long step_elapsed = getElapsedTime(cycle->runtime.pattern_step_start);
    
    if (step_elapsed >= current_step->duration_ms) {
        // Move to next pattern step
        cycle->runtime.pattern_step = (cycle->runtime.pattern_step + 1) % cycle->config.pattern_length;
        cycle->runtime.pattern_step_start = current_time;
        cycle->runtime.pattern_step_active = cycle->config.pattern[cycle->runtime.pattern_step].active;
        return true;
    }
    
    return false;
}

void setCycleEnabled(int cycle_id, bool enabled) {
    if (cycle_id < 0 || cycle_id >= (int)cycle_count) {
        return;
    }
    
    cycles[cycle_id].config.enabled = enabled;
    if (enabled && cycles[cycle_id].runtime.state == CYCLE_STATE_INACTIVE) {
        cycles[cycle_id].runtime.state = CYCLE_STATE_ACTIVE;
    } else if (!enabled) {
        cycles[cycle_id].runtime.state = CYCLE_STATE_INACTIVE;
    }
}

void setCyclePaused(int cycle_id, bool paused) {
    if (cycle_id < 0 || cycle_id >= (int)cycle_count) {
        return;
    }
    
    if (paused) {
        cycles[cycle_id].runtime.state = CYCLE_STATE_PAUSED;
    } else if (cycles[cycle_id].config.enabled) {
        cycles[cycle_id].runtime.state = CYCLE_STATE_ACTIVE;
    }
}

cycle_state_t getCycleState(int cycle_id) {
    if (cycle_id < 0 || cycle_id >= (int)cycle_count) {
        return CYCLE_STATE_INACTIVE;
    }
    
    return cycles[cycle_id].runtime.state;
}

const cycle_runtime_t* getCycleStats(int cycle_id) {
    if (cycle_id < 0 || cycle_id >= (int)cycle_count) {
        return nullptr;
    }
    
    return &cycles[cycle_id].runtime;
}

void resetCycleStats(int cycle_id) {
    if (cycle_id < 0 || cycle_id >= (int)cycle_count) {
        return;
    }
    
    cycles[cycle_id].runtime.execution_count = 0;
    cycles[cycle_id].runtime.error_count = 0;
    cycles[cycle_id].runtime.total_execution_time = 0;
    cycles[cycle_id].runtime.max_execution_time = 0;
}

void printCycleManagerStats() {
    Serial.println("\n=== Cycle Manager Statistics ===");
    Serial.printf("Total cycles: %d\n", cycle_count);
    Serial.printf("Total executions: %lu\n", total_cycles_executed);
    Serial.printf("Total execution time: %lu ms\n", total_execution_time);
    Serial.printf("Last update: %lu ms ago\n", getElapsedTime(last_manager_update));
    
    Serial.println("\nCycle Summary:");
    for (size_t i = 0; i < cycle_count; i++) {
        cycle_t* cycle = &cycles[i];
        Serial.printf("  %s: %s, %lu executions, %lu errors\n",
                      cycle->config.name,
                      getCycleStateString(cycle->runtime.state),
                      cycle->runtime.execution_count,
                      cycle->runtime.error_count);
    }
}

void printCycleStats(int cycle_id) {
    if (cycle_id < 0 || cycle_id >= (int)cycle_count) {
        Serial.printf("Invalid cycle ID: %d\n", cycle_id);
        return;
    }
    
    cycle_t* cycle = &cycles[cycle_id];
    Serial.printf("\n=== Cycle '%s' Statistics ===\n", cycle->config.name);
    Serial.printf("State: %s\n", getCycleStateString(cycle->runtime.state));
    Serial.printf("Priority: %d\n", cycle->config.priority);
    Serial.printf("Mode: %s\n", getCycleModeString(cycle->config.mode));
    Serial.printf("Enabled: %s\n", cycle->config.enabled ? "Yes" : "No");
    Serial.printf("Executions: %lu\n", cycle->runtime.execution_count);
    Serial.printf("Errors: %lu\n", cycle->runtime.error_count);
    Serial.printf("Total execution time: %lu ms\n", cycle->runtime.total_execution_time);
    Serial.printf("Max execution time: %lu ms\n", cycle->runtime.max_execution_time);
    if (cycle->runtime.execution_count > 0) {
        Serial.printf("Average execution time: %lu ms\n", 
                      cycle->runtime.total_execution_time / cycle->runtime.execution_count);
    }
}

const char* getCycleStateString(cycle_state_t state) {
    switch (state) {
        case CYCLE_STATE_INACTIVE: return "INACTIVE";
        case CYCLE_STATE_ACTIVE: return "ACTIVE";
        case CYCLE_STATE_PAUSED: return "PAUSED";
        case CYCLE_STATE_ERROR: return "ERROR";
        case CYCLE_STATE_COMPLETED: return "COMPLETED";
        default: return "UNKNOWN";
    }
}

const char* getCycleModeString(cycle_mode_t mode) {
    switch (mode) {
        case CYCLE_MODE_INTERVAL: return "INTERVAL";
        case CYCLE_MODE_TIMEOUT: return "TIMEOUT";
        case CYCLE_MODE_CONDITION: return "CONDITION";
        case CYCLE_MODE_PATTERN: return "PATTERN";
        case CYCLE_MODE_CIRCULAR_BUFFER: return "CIRCULAR_BUFFER";
        case CYCLE_MODE_STATE_MACHINE: return "STATE_MACHINE";
        default: return "UNKNOWN";
    }
}

// ===================================================================
// CONVENIENCE FUNCTIONS
// ===================================================================

int registerIntervalCycle(const char* name, uint32_t interval_ms, 
                         std::function<void()> execute, 
                         cycle_priority_t priority) {
    cycle_config_t config = {};
    config.name = name;
    config.mode = CYCLE_MODE_INTERVAL;
    config.priority = priority;
    config.interval_ms = interval_ms;
    config.execute = execute;
    config.enabled = true;
    config.one_shot = false;
    
    return registerCycle(config);
}

int registerTimeoutCycle(const char* name, uint32_t timeout_ms, 
                        std::function<void()> execute,
                        cycle_priority_t priority) {
    cycle_config_t config = {};
    config.name = name;
    config.mode = CYCLE_MODE_TIMEOUT;
    config.priority = priority;
    config.timeout_ms = timeout_ms;
    config.execute = execute;
    config.enabled = true;
    config.one_shot = true;
    
    return registerCycle(config);
}

int registerConditionCycle(const char* name, std::function<bool()> condition,
                          std::function<void()> execute,
                          cycle_priority_t priority) {
    cycle_config_t config = {};
    config.name = name;
    config.mode = CYCLE_MODE_CONDITION;
    config.priority = priority;
    config.condition = condition;
    config.execute = execute;
    config.enabled = true;
    config.one_shot = false;
    
    return registerCycle(config);
}

int registerPatternCycle(const char* name, pattern_step_t* pattern, size_t pattern_length,
                        std::function<void()> execute,
                        cycle_priority_t priority) {
    cycle_config_t config = {};
    config.name = name;
    config.mode = CYCLE_MODE_PATTERN;
    config.priority = priority;
    config.pattern = pattern;
    config.pattern_length = pattern_length;
    config.execute = execute;
    config.enabled = true;
    config.one_shot = false;
    
    return registerCycle(config);
}

int registerCircularBufferCycle(const char* name, circular_buffer_config_t* buffer_config,
                               std::function<void()> execute,
                               cycle_priority_t priority) {
    cycle_config_t config = {};
    config.name = name;
    config.mode = CYCLE_MODE_CIRCULAR_BUFFER;
    config.priority = priority;
    config.buffer_config = buffer_config;
    config.execute = execute;
    config.enabled = true;
    config.one_shot = false;
    
    return registerCycle(config);
} 