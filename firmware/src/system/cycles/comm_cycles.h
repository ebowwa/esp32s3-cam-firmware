#ifndef COMM_CYCLES_H
#define COMM_CYCLES_H

namespace CommCycles {
    void initialize();
    void registerDataTransmissionCycle();
    void registerConnectionMonitorCycle();
    
    extern int data_transmission_cycle_id;
    extern int connection_monitor_cycle_id;
}

#endif // COMM_CYCLES_H 