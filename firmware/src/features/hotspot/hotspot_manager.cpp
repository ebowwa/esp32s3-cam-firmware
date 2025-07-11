#include "hotspot_manager.h"

// ===================================================================
// GLOBAL HOTSPOT MANAGER STATE
// ===================================================================

// Global hotspot statistics
hotspot_stats_t hotspotStats = {
    .status = HOTSPOT_DISABLED,
    .connected_clients = 0,
    .max_clients = HOTSPOT_MAX_CONNECTIONS,
    .start_time = 0,
    .last_activity = 0,
    .total_connections = 0,
    .data_sent_bytes = 0,
    .data_received_bytes = 0,
    .ssid = "",
    .ip_address = ""
};

// Web server instance
WebServer webServer(HOTSPOT_WEB_PORT);

// Hotspot enabled flag
bool hotspotEnabled = false;

// ===================================================================
// HOTSPOT MANAGER IMPLEMENTATION
// ===================================================================

// All functions are implemented as static inline in the header file
// for performance reasons and to maintain compatibility with existing
// ESP32 Arduino Core 2.0.17 patterns.

// This implementation file only contains the global variable definitions
// and any non-inline functions that may be added in the future. 