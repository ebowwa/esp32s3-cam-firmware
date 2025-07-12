#ifndef LED_CYCLES_H
#define LED_CYCLES_H

namespace LEDCycles {
    void initialize();
    void registerLEDUpdateCycle();
    void registerPatternCycles();
    
    extern int led_update_cycle_id;
}

#endif // LED_CYCLES_H 