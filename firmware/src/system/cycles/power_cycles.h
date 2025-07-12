#ifndef POWER_CYCLES_H
#define POWER_CYCLES_H

namespace PowerCycles {
    void initialize();
    void registerBatteryUpdateCycle();
    void registerPowerStatsCycle();
    void registerSleepManagementCycle();
    void registerMemoryMonitorCycle();
    void registerMemoryLeakCheckCycle();
    void registerHotspotMonitorCycle();
    
    extern int battery_update_cycle_id;
    extern int power_stats_cycle_id;
    extern int sleep_management_cycle_id;
    extern int memory_monitor_cycle_id;
    extern int memory_leak_check_cycle_id;
    extern int hotspot_monitor_cycle_id;
}

#endif // POWER_CYCLES_H 