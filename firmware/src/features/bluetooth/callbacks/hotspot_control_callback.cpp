#include "hotspot_control_callback.h"
#include "../characteristics/ble_characteristics.h"

// Hotspot Control Callback Implementation
void HotspotControlCallback::onWrite(BLECharacteristic *characteristic) {
    Serial.printf("Hotspot control write received, length: %d\n", characteristic->getLength());
    
    // Record BLE command received
    // recordBLECommandReceived();  // DISABLED: Function not defined and causes BLE interference
    
    if (characteristic->getLength() == 1) {
        uint8_t value = characteristic->getData()[0];
        Serial.printf("Hotspot control value: %d (0x%02X)\n", value, value);
        handleHotspotControl(value);
    } else {
        Serial.println("Invalid hotspot control message length");
    }
}

// Hotspot Control Handler - DISABLED: Causes BLE interference
void handleHotspotControl(uint8_t controlValue) {
    Serial.printf("Hotspot control received: %d (DISABLED)\n", controlValue);
    // All hotspot functionality disabled to prevent WiFi/BLE interference
    /*
    switch (controlValue) {
        case 0x00: // Stop hotspot
            Serial.println("Stopping hotspot via BLE command");
            stopHotspot();
            break;
            
        case 0x01: // Start hotspot
            Serial.println("Starting hotspot via BLE command");
            startHotspot();
            break;
            
        case 0x02: // Toggle hotspot
            Serial.println("Toggling hotspot via BLE command");
            toggleHotspot();
            break;
            
        case 0x03: // Get hotspot status
            Serial.println("Hotspot status requested via BLE");
            updateHotspotStatus();
            break;
            
                 default:
             Serial.printf("Unknown hotspot control value: 0x%02X\n", controlValue);
             break;
     }
     */
}

// Update hotspot status characteristic - DISABLED: Causes BLE interference
void updateHotspotStatus() {
    if (!hotspotStatusCharacteristic || !bleConnected) return;
    
    // Send minimal status to indicate hotspot is disabled
    uint8_t statusData[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // All zeros = disabled
    hotspotStatusCharacteristic->setValue(statusData, 8);
    hotspotStatusCharacteristic->notify();
    
    Serial.println("Hotspot status updated: DISABLED (prevents BLE interference)");
    
    /*
    // Original hotspot status code disabled to prevent WiFi/BLE interference
    hotspot_stats_t stats = getHotspotStats();
    
    // Create extended status packet with BLE data
    uint8_t statusData[64];
    int pos = 0;
    
    // Basic hotspot status
    statusData[pos++] = (uint8_t)stats.status;
    statusData[pos++] = stats.connected_clients;
    statusData[pos++] = stats.max_clients;
    
    // BLE integration data
    statusData[pos++] = stats.ble_connected ? 1 : 0;
    statusData[pos++] = stats.active_interfaces;
    
    // Data statistics (4 bytes each, little endian)
    uint32_t total_data = stats.total_data_all_interfaces;
    statusData[pos++] = total_data & 0xFF;
    statusData[pos++] = (total_data >> 8) & 0xFF;
    statusData[pos++] = (total_data >> 16) & 0xFF;
    statusData[pos++] = (total_data >> 24) & 0xFF;
    
    uint32_t ble_data = stats.ble_data_transmitted;
    statusData[pos++] = ble_data & 0xFF;
    statusData[pos++] = (ble_data >> 8) & 0xFF;
    statusData[pos++] = (ble_data >> 16) & 0xFF;
    statusData[pos++] = (ble_data >> 24) & 0xFF;
    
    // Add SSID length and SSID (up to 20 chars)
    uint8_t ssidLen = min(stats.ssid.length(), (unsigned int)20);
    statusData[pos++] = ssidLen;
    if (ssidLen > 0) {
        memcpy(&statusData[pos], stats.ssid.c_str(), ssidLen);
        pos += ssidLen;
    }
    
    // Add IP address length and IP (up to 15 chars)
    uint8_t ipLen = min(stats.ip_address.length(), (unsigned int)15);
    statusData[pos++] = ipLen;
    if (ipLen > 0) {
        memcpy(&statusData[pos], stats.ip_address.c_str(), ipLen);
        pos += ipLen;
    }
    
    hotspotStatusCharacteristic->setValue(statusData, pos);
    hotspotStatusCharacteristic->notify();
    
    // Record BLE data transmission for this status update
    recordBLEDataTransmission(pos);
    
    Serial.printf("Hotspot status updated: Status=%d, Clients=%d/%d, SSID=%s, IP=%s, BLE=%s, Total=%u bytes\n",
                  stats.status, stats.connected_clients, stats.max_clients,
                  stats.ssid.c_str(), stats.ip_address.c_str(),
                  stats.ble_connected ? "Connected" : "Disconnected",
                  stats.total_data_all_interfaces);
    */
} 