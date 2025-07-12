#include "led_cycles.h"
#include "cycle_manager.h"
#include "../../hal/led/led_manager.h"
#include <Arduino.h>

// ===================================================================
// LED PATTERN CYCLE MANAGER
// ===================================================================

namespace LEDCycles {
    int led_update_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing LED Cycles...");
        registerLEDUpdateCycle();
        registerPatternCycles();
    }
    
    void registerLEDUpdateCycle() {
        led_update_cycle_id = registerIntervalCycle(
            "LEDUpdate",
            20, // 20ms for smooth LED updates
            []() {
                updateLed();
            },
            CYCLE_PRIORITY_CRITICAL
        );
    }
    
    void registerPatternCycles() {
        // LED patterns are handled by the main LED update cycle
        // Individual patterns can be registered as needed
    }
} 