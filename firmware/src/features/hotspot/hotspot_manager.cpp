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

void initializeHotspotManager() {
    Serial.println("Initializing Hotspot Manager...");
    
    // Initialize statistics
    hotspotStats.status = HOTSPOT_DISABLED;
    hotspotStats.connected_clients = 0;
    hotspotStats.max_clients = HOTSPOT_MAX_CONNECTIONS;
    hotspotStats.start_time = 0;
    hotspotStats.last_activity = 0;
    hotspotStats.total_connections = 0;
    hotspotStats.data_sent_bytes = 0;
    hotspotStats.data_received_bytes = 0;
    hotspotStats.ssid = "";
    hotspotStats.ip_address = "";
    
    // Initialize BLE integration data
    hotspotStats.ble_connected = false;
    hotspotStats.ble_connection_start = 0;
    hotspotStats.ble_data_transmitted = 0;
    hotspotStats.ble_commands_received = 0;
    hotspotStats.ble_client_info = "";
    hotspotStats.total_data_all_interfaces = 0;
    hotspotStats.active_interfaces = 0;
    hotspotStats.data_rate_mbps = 0.0;
    
    hotspotEnabled = false;
    
    Serial.println("Hotspot Manager initialized");
}

String generateHotspotSSID() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String ssid = HOTSPOT_SSID_PREFIX + mac.substring(8); // Last 4 chars of MAC
    return ssid;
}

bool startHotspot() {
    if (hotspotEnabled) {
        Serial.println("Hotspot already running");
        return true;
    }
    
    Serial.println("Starting hotspot...");
    hotspotStats.status = HOTSPOT_STARTING;
    
    // Generate unique SSID
    String ssid = generateHotspotSSID();
    
    // Configure access point
    WiFi.mode(WIFI_AP);
    
    // Set custom IP configuration
    if (!WiFi.softAPConfig(HOTSPOT_IP_ADDRESS, HOTSPOT_GATEWAY, HOTSPOT_SUBNET)) {
        Serial.println("Failed to configure hotspot IP");
        hotspotStats.status = HOTSPOT_ERROR;
        return false;
    }
    
    // Start access point
    bool success = WiFi.softAP(
        ssid.c_str(),
        HOTSPOT_PASSWORD,
        HOTSPOT_CHANNEL,
        HOTSPOT_HIDDEN,
        HOTSPOT_MAX_CONNECTIONS
    );
    
    if (success) {
        hotspotEnabled = true;
        hotspotStats.status = HOTSPOT_ACTIVE;
        hotspotStats.start_time = millis();
        hotspotStats.last_activity = millis();
        hotspotStats.ssid = ssid;
        hotspotStats.ip_address = WiFi.softAPIP().toString();
        
        Serial.printf("✅ Hotspot started successfully!\n");
        Serial.printf("   SSID: %s\n", ssid.c_str());
        Serial.printf("   Password: %s\n", HOTSPOT_PASSWORD);
        Serial.printf("   IP: %s\n", hotspotStats.ip_address.c_str());
        Serial.printf("   Max Clients: %d\n", HOTSPOT_MAX_CONNECTIONS);
        
        return true;
    } else {
        Serial.println("❌ Failed to start hotspot");
        hotspotStats.status = HOTSPOT_ERROR;
        return false;
    }
}

void stopHotspot() {
    if (!hotspotEnabled) {
        Serial.println("Hotspot already stopped");
        return;
    }
    
    Serial.println("Stopping hotspot...");
    
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    
    hotspotEnabled = false;
    hotspotStats.status = HOTSPOT_DISABLED;
    hotspotStats.connected_clients = 0;
    hotspotStats.ssid = "";
    hotspotStats.ip_address = "";
    
    Serial.println("Hotspot stopped");
}

void updateHotspotStats() {
    if (!hotspotEnabled) return;
    
    // Update connected clients count
    uint8_t current_clients = WiFi.softAPgetStationNum();
    
    if (current_clients != hotspotStats.connected_clients) {
        if (current_clients > hotspotStats.connected_clients) {
            hotspotStats.total_connections++;
            Serial.printf("Client connected (total: %d)\n", current_clients);
        } else {
            Serial.printf("Client disconnected (total: %d)\n", current_clients);
        }
        
        hotspotStats.connected_clients = current_clients;
        hotspotStats.last_activity = millis();
    }
}

hotspot_status_t getHotspotStatus() {
    return hotspotStats.status;
}

hotspot_stats_t getHotspotStats() {
    return hotspotStats;
}

bool isHotspotActive() {
    return hotspotEnabled && (hotspotStats.status == HOTSPOT_ACTIVE);
}

uint8_t getConnectedClients() {
    return hotspotStats.connected_clients;
}

void printHotspotStats() {
    Serial.println("=== Hotspot Statistics ===");
    Serial.printf("Status: %s\n", 
        hotspotStats.status == HOTSPOT_DISABLED ? "DISABLED" :
        hotspotStats.status == HOTSPOT_STARTING ? "STARTING" :
        hotspotStats.status == HOTSPOT_ACTIVE ? "ACTIVE" : "ERROR");
    
    if (hotspotStats.status == HOTSPOT_ACTIVE) {
        Serial.printf("SSID: %s\n", hotspotStats.ssid.c_str());
        Serial.printf("IP Address: %s\n", hotspotStats.ip_address.c_str());
        Serial.printf("Connected Clients: %d/%d\n", 
                     hotspotStats.connected_clients, hotspotStats.max_clients);
        Serial.printf("Total Connections: %u\n", hotspotStats.total_connections);
        Serial.printf("Uptime: %lu ms\n", millis() - hotspotStats.start_time);
        Serial.printf("Last Activity: %lu ms ago\n", 
                     millis() - hotspotStats.last_activity);
    }
    Serial.println("==========================");
}

bool toggleHotspot() {
    if (hotspotEnabled) {
        stopHotspot();
        return false;
    } else {
        return startHotspot();
    }
}

unsigned long getHotspotUptime() {
    if (!hotspotEnabled || hotspotStats.start_time == 0) {
        return 0;
    }
    return (millis() - hotspotStats.start_time) / 1000;
}

bool shouldAutoStartHotspot() {
    // Auto-start conditions (customize as needed):
    // - No BLE connection for extended period
    // - Device in standalone mode
    // - User button pressed (if implemented)
    
    // For now, return false - manual control only
    return false;
}

void handleHotspotAutoManagement() {
    if (!hotspotEnabled && shouldAutoStartHotspot()) {
        Serial.println("Auto-starting hotspot...");
        startHotspot();
    }
}

String getHotspotConnectionInfo() {
    if (!hotspotEnabled) {
        return "Hotspot: Disabled";
    }
    
    String info = "Hotspot: " + hotspotStats.ssid + "\n";
    info += "IP: " + hotspotStats.ip_address + "\n";
    info += "Clients: " + String(hotspotStats.connected_clients) + "/" + String(hotspotStats.max_clients);
    
    return info;
}

// ===================================================================
// BLE INTEGRATION FUNCTIONS
// ===================================================================

void updateCombinedStatistics() {
    // Update active interfaces
    hotspotStats.active_interfaces = 0;
    if (hotspotStats.ble_connected) {
        hotspotStats.active_interfaces |= 0x01;
    }
    if (hotspotEnabled && hotspotStats.connected_clients > 0) {
        hotspotStats.active_interfaces |= 0x02;
    }
    
    // Calculate combined data rate (simplified)
    unsigned long uptime = millis() - min(hotspotStats.start_time, hotspotStats.ble_connection_start);
    if (uptime > 0) {
        hotspotStats.data_rate_mbps = (hotspotStats.total_data_all_interfaces * 8.0) / (uptime * 1000.0); // Mbps
    }
    
    // Update total data from all interfaces
    hotspotStats.total_data_all_interfaces = hotspotStats.data_sent_bytes + 
                                           hotspotStats.data_received_bytes + 
                                           hotspotStats.ble_data_transmitted;
}

void updateBLEConnectionStatus(bool connected, const String& client_info) {
    bool was_connected = hotspotStats.ble_connected;
    hotspotStats.ble_connected = connected;
    
    if (connected && !was_connected) {
        hotspotStats.ble_connection_start = millis();
        hotspotStats.ble_client_info = client_info;
        hotspotStats.active_interfaces |= 0x01; // Set BLE bit
        Serial.println("BLE connection established - updating hotspot stats");
    } else if (!connected && was_connected) {
        hotspotStats.ble_client_info = "";
        hotspotStats.active_interfaces &= ~0x01; // Clear BLE bit
        Serial.println("BLE connection lost - updating hotspot stats");
    }
    
    updateCombinedStatistics();
}

void recordBLEDataTransmission(uint32_t bytes_sent) {
    hotspotStats.ble_data_transmitted += bytes_sent;
    hotspotStats.total_data_all_interfaces += bytes_sent;
    hotspotStats.last_activity = millis();
    
    updateCombinedStatistics();
}

void recordBLECommandReceived() {
    hotspotStats.ble_commands_received++;
    hotspotStats.last_activity = millis();
}

String getComprehensiveConnectionInfo() {
    String info = "";
    
    // BLE Status
    if (hotspotStats.ble_connected) {
        unsigned long ble_uptime = (millis() - hotspotStats.ble_connection_start) / 1000;
        info += "BLE: Connected (" + String(ble_uptime) + "s)\n";
        info += "BLE Data: " + String(hotspotStats.ble_data_transmitted) + " bytes\n";
        info += "BLE Commands: " + String(hotspotStats.ble_commands_received) + "\n";
        if (hotspotStats.ble_client_info.length() > 0) {
            info += "BLE Client: " + hotspotStats.ble_client_info + "\n";
        }
    } else {
        info += "BLE: Disconnected\n";
    }
    
    // Hotspot Status
    if (hotspotEnabled) {
        info += "Hotspot: " + hotspotStats.ssid + "\n";
        info += "WiFi Clients: " + String(hotspotStats.connected_clients) + "/" + String(hotspotStats.max_clients) + "\n";
        info += "Hotspot IP: " + hotspotStats.ip_address + "\n";
    } else {
        info += "Hotspot: Disabled\n";
    }
    
    // Combined Statistics
    info += "Total Data: " + String(hotspotStats.total_data_all_interfaces) + " bytes\n";
    info += "Data Rate: " + String(hotspotStats.data_rate_mbps, 2) + " Mbps\n";
    info += "Active Interfaces: ";
    if (hotspotStats.active_interfaces & 0x01) info += "BLE ";
    if (hotspotStats.active_interfaces & 0x02) info += "WiFi ";
    if (hotspotStats.active_interfaces == 0) info += "None";
    
    return info;
}

bool shouldAutoStartHotspotBasedOnBLE() {
    // Auto-start hotspot if:
    // 1. BLE has been connected for a while (indicates active use)
    // 2. BLE is receiving commands (user is actively controlling)
    // 3. BLE data transmission is high (might benefit from WiFi offload)
    
    if (!hotspotStats.ble_connected) {
        return false;
    }
    
    unsigned long ble_uptime = millis() - hotspotStats.ble_connection_start;
    
    // Auto-start conditions
    bool long_ble_session = ble_uptime > 300000; // 5 minutes
    bool active_commands = hotspotStats.ble_commands_received > 10;
    bool high_data_usage = hotspotStats.ble_data_transmitted > 1000000; // 1MB
    
    return long_ble_session && (active_commands || high_data_usage);
}

void handleEnhancedHotspotAutoManagement() {
    // Original auto-management
    handleHotspotAutoManagement();
    
    // BLE-based auto-management
    if (!hotspotEnabled && shouldAutoStartHotspotBasedOnBLE()) {
        Serial.println("Auto-starting hotspot based on BLE activity...");
        startHotspot();
    }
}

void printComprehensiveHotspotStats() {
    Serial.println("=== Comprehensive Connection Statistics ===");
    
    // BLE Statistics
    Serial.printf("BLE Connection: %s\n", hotspotStats.ble_connected ? "Connected" : "Disconnected");
    if (hotspotStats.ble_connected) {
        unsigned long ble_uptime = (millis() - hotspotStats.ble_connection_start) / 1000;
        Serial.printf("BLE Uptime: %lu seconds\n", ble_uptime);
        Serial.printf("BLE Data Transmitted: %u bytes\n", hotspotStats.ble_data_transmitted);
        Serial.printf("BLE Commands Received: %u\n", hotspotStats.ble_commands_received);
        if (hotspotStats.ble_client_info.length() > 0) {
            Serial.printf("BLE Client: %s\n", hotspotStats.ble_client_info.c_str());
        }
    }
    
    // Hotspot Statistics
    Serial.printf("Hotspot Status: %s\n", 
        hotspotStats.status == HOTSPOT_DISABLED ? "DISABLED" :
        hotspotStats.status == HOTSPOT_STARTING ? "STARTING" :
        hotspotStats.status == HOTSPOT_ACTIVE ? "ACTIVE" : "ERROR");
    
    if (hotspotStats.status == HOTSPOT_ACTIVE) {
        Serial.printf("Hotspot SSID: %s\n", hotspotStats.ssid.c_str());
        Serial.printf("Hotspot IP: %s\n", hotspotStats.ip_address.c_str());
        Serial.printf("WiFi Clients: %d/%d\n", hotspotStats.connected_clients, hotspotStats.max_clients);
        Serial.printf("WiFi Connections: %u total\n", hotspotStats.total_connections);
    }
    
    // Combined Statistics
    Serial.printf("Total Data (All Interfaces): %u bytes\n", hotspotStats.total_data_all_interfaces);
    Serial.printf("Combined Data Rate: %.2f Mbps\n", hotspotStats.data_rate_mbps);
    Serial.printf("Active Interfaces: ");
    if (hotspotStats.active_interfaces & 0x01) Serial.print("BLE ");
    if (hotspotStats.active_interfaces & 0x02) Serial.print("WiFi ");
    if (hotspotStats.active_interfaces == 0) Serial.print("None");
    Serial.println();
    
    Serial.printf("Last Activity: %lu ms ago\n", millis() - hotspotStats.last_activity);
    Serial.println("==========================================");
} 