#include "cycle_manager.h"
#include "charging_cycles.h"
#include "led_cycles.h"
#include "power_cycles.h"
#include "data_cycles.h"
#include "comm_cycles.h"
#include <Arduino.h>

// ===================================================================
// SPECIALIZED CYCLES INITIALIZATION
// ===================================================================

void initializeSpecializedCycles() {
    Serial.println("Initializing all specialized cycles...");
    
    // Initialize each cycle module
    ChargingCycles::initialize();
    LEDCycles::initialize();
    PowerCycles::initialize();
    DataCycles::initialize();
    CommCycles::initialize();
    
    Serial.println("All specialized cycles initialized");
} 