// Test sketch for Centralized Cycle Manager
// This demonstrates the unified cycle management system

#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE

#include "../firmware/src/system/cycles/cycle_manager.h"
#include "../firmware/src/hal/xiao_esp32s3_constants.h"

// Test cycle IDs
int test_interval_cycle_id = -1;
int test_condition_cycle_id = -1;
int test_pattern_cycle_id = -1;
int test_timeout_cycle_id = -1;

// Test variables
volatile int counter = 0;
volatile bool condition_met = false;
volatile unsigned long last_pattern_time = 0;

// Test pattern for LED-like behavior
pattern_step_t test_pattern[] = {
    {500, 255, true},   // 500ms ON
    {500, 0, false},    // 500ms OFF
    {200, 255, true},   // 200ms ON
    {200, 0, false},    // 200ms OFF
    {200, 255, true},   // 200ms ON
    {1000, 0, false}    // 1000ms OFF
};

void setup() {
    Serial.begin(XIAO_ESP32S3_SERIAL_BAUD_RATE);
    Serial.println("Testing Centralized Cycle Manager...");
    Serial.println("====================================");
    
    // Initialize cycle manager
    initializeCycleManager();
    
    // Test 1: Interval-based cycle
    Serial.println("\n1. Registering interval cycle (2 seconds)...");
    test_interval_cycle_id = registerIntervalCycle(
        "TestInterval",
        2000, // 2 seconds
        []() {
            counter++;
            Serial.printf("Interval cycle executed: counter = %d\n", counter);
        },
        CYCLE_PRIORITY_NORMAL
    );
    
    // Test 2: Condition-based cycle
    Serial.println("2. Registering condition cycle...");
    test_condition_cycle_id = registerConditionCycle(
        "TestCondition",
        []() {
            return condition_met;
        },
        []() {
            Serial.println("Condition cycle executed!");
            condition_met = false; // Reset condition
        },
        CYCLE_PRIORITY_HIGH
    );
    
    // Test 3: Pattern-based cycle
    Serial.println("3. Registering pattern cycle...");
    test_pattern_cycle_id = registerPatternCycle(
        "TestPattern",
        test_pattern,
        sizeof(test_pattern) / sizeof(pattern_step_t),
        []() {
            // Get current pattern step
            const cycle_runtime_t* stats = getCycleStats(test_pattern_cycle_id);
            if (stats) {
                pattern_step_t* current_step = &test_pattern[stats->pattern_step];
                Serial.printf("Pattern step %d: value=%d, active=%s\n", 
                             stats->pattern_step, 
                             current_step->value,
                             current_step->active ? "true" : "false");
                last_pattern_time = millis();
            }
        },
        CYCLE_PRIORITY_NORMAL
    );
    
    // Test 4: Timeout-based cycle (one-shot)
    Serial.println("4. Registering timeout cycle (5 seconds)...");
    test_timeout_cycle_id = registerTimeoutCycle(
        "TestTimeout",
        5000, // 5 seconds
        []() {
            Serial.println("Timeout cycle executed (one-shot)!");
        },
        CYCLE_PRIORITY_LOW
    );
    
    Serial.println("\nAll cycles registered. Starting main loop...");
    Serial.println("Type 'c' to trigger condition cycle");
    Serial.println("Type 's' to show statistics");
    Serial.println("Type 'r' to reset statistics");
    Serial.println("Type 'p' to pause/resume interval cycle");
}

void loop() {
    // Update all cycles
    updateCycles();
    
    // Handle serial input
    if (Serial.available()) {
        char cmd = Serial.read();
        handleSerialCommand(cmd);
    }
    
    // Trigger condition periodically for testing
    static unsigned long lastConditionTrigger = 0;
    if (millis() - lastConditionTrigger > 7000) { // Every 7 seconds
        condition_met = true;
        lastConditionTrigger = millis();
    }
    
    // Print cycle manager stats every 30 seconds
    static unsigned long lastStatsTime = 0;
    if (millis() - lastStatsTime > 30000) {
        printCycleManagerStats();
        lastStatsTime = millis();
    }
    
    delay(10); // Small delay
}

void handleSerialCommand(char cmd) {
    switch (cmd) {
        case 'c':
        case 'C':
            Serial.println("Triggering condition cycle...");
            condition_met = true;
            break;
            
        case 's':
        case 'S':
            Serial.println("=== CYCLE STATISTICS ===");
            printCycleManagerStats();
            Serial.println("\nIndividual cycle details:");
            for (int i = 0; i < 4; i++) {
                printCycleStats(i);
            }
            break;
            
        case 'r':
        case 'R':
            Serial.println("Resetting cycle statistics...");
            for (int i = 0; i < 4; i++) {
                resetCycleStats(i);
            }
            counter = 0;
            Serial.println("Statistics reset!");
            break;
            
        case 'p':
        case 'P':
            {
                static bool paused = false;
                paused = !paused;
                setCyclePaused(test_interval_cycle_id, paused);
                Serial.printf("Interval cycle %s\n", paused ? "PAUSED" : "RESUMED");
            }
            break;
            
        case 'e':
        case 'E':
            {
                static bool enabled = true;
                enabled = !enabled;
                setCycleEnabled(test_interval_cycle_id, enabled);
                Serial.printf("Interval cycle %s\n", enabled ? "ENABLED" : "DISABLED");
            }
            break;
            
        case 't':
        case 'T':
            testCyclePerformance();
            break;
            
        case 'h':
        case 'H':
        case '?':
            printHelp();
            break;
            
        default:
            Serial.printf("Unknown command: %c\n", cmd);
            break;
    }
}

void testCyclePerformance() {
    Serial.println("\n=== PERFORMANCE TEST ===");
    
    unsigned long start_time = millis();
    unsigned long cycles_before = total_cycles_executed;
    
    // Run for 5 seconds
    while (millis() - start_time < 5000) {
        updateCycles();
        delay(1);
    }
    
    unsigned long end_time = millis();
    unsigned long cycles_after = total_cycles_executed;
    
    Serial.printf("Performance test results:\n");
    Serial.printf("  Duration: %lu ms\n", end_time - start_time);
    Serial.printf("  Cycles executed: %lu\n", cycles_after - cycles_before);
    Serial.printf("  Cycles per second: %.2f\n", 
                  (float)(cycles_after - cycles_before) / ((end_time - start_time) / 1000.0));
    Serial.printf("  Total execution time: %lu ms\n", total_execution_time);
}

void printHelp() {
    Serial.println("\n=== AVAILABLE COMMANDS ===");
    Serial.println("c - Trigger condition cycle");
    Serial.println("s - Show cycle statistics");
    Serial.println("r - Reset cycle statistics");
    Serial.println("p - Pause/Resume interval cycle");
    Serial.println("e - Enable/Disable interval cycle");
    Serial.println("t - Run performance test");
    Serial.println("h - Show this help");
} 