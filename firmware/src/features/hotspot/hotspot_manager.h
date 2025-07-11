#ifndef HOTSPOT_MANAGER_H
#define HOTSPOT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "../../hal/xiao_esp32s3_constants.h"

// ===================================================================
// STANDALONE HOTSPOT MANAGER FOR ESP32S3
// ===================================================================
//
// Provides WiFi Access Point functionality alongside existing BLE
// Creates a standalone network for direct device access
//

// Hotspot Configuration
#define HOTSPOT_SSID_PREFIX "ESP32CAM-"
#define HOTSPOT_PASSWORD "openglass123"
#define HOTSPOT_CHANNEL 1
#define HOTSPOT_MAX_CONNECTIONS 4
#define HOTSPOT_HIDDEN false
#define HOTSPOT_IP_ADDRESS IPAddress(192, 168, 4, 1)
#define HOTSPOT_GATEWAY IPAddress(192, 168, 4, 1)
#define HOTSPOT_SUBNET IPAddress(255, 255, 255, 0)
#define HOTSPOT_WEB_PORT 80

// Hotspot Status
typedef enum {
    HOTSPOT_DISABLED,
    HOTSPOT_STARTING,
    HOTSPOT_ACTIVE,
    HOTSPOT_ERROR
} hotspot_status_t;

// Hotspot Statistics with BLE Integration
typedef struct {
    hotspot_status_t status;
    uint8_t connected_clients;
    uint8_t max_clients;
    unsigned long start_time;
    unsigned long last_activity;
    uint32_t total_connections;
    uint32_t data_sent_bytes;
    uint32_t data_received_bytes;
    String ssid;
    String ip_address;
    
    // BLE Connection Data Integration
    bool ble_connected;
    unsigned long ble_connection_start;
    uint32_t ble_data_transmitted;
    uint32_t ble_commands_received;
    String ble_client_info;
    
    // Combined Statistics
    uint32_t total_data_all_interfaces;
    uint8_t active_interfaces;  // Bit flags: 0x01=BLE, 0x02=WiFi
    float data_rate_mbps;
} hotspot_stats_t;

// Global hotspot state
extern hotspot_stats_t hotspotStats;
extern WebServer webServer;
extern bool hotspotEnabled;

// ===================================================================
// FUNCTION DECLARATIONS
// ===================================================================

/**
 * Initialize hotspot manager
 */
void initializeHotspotManager();

/**
 * Generate unique SSID based on device MAC
 */
String generateHotspotSSID();

/**
 * Start hotspot access point
 */
bool startHotspot();

/**
 * Stop hotspot access point
 */
void stopHotspot();

/**
 * Update hotspot statistics
 */
void updateHotspotStats();

/**
 * Get hotspot status
 */
hotspot_status_t getHotspotStatus();

/**
 * Get hotspot statistics
 */
hotspot_stats_t getHotspotStats();

/**
 * Check if hotspot is active
 */
bool isHotspotActive();

/**
 * Get number of connected clients
 */
uint8_t getConnectedClients();

/**
 * Print hotspot statistics
 */
void printHotspotStats();

/**
 * Toggle hotspot on/off
 */
bool toggleHotspot();

/**
 * Get hotspot uptime in seconds
 */
unsigned long getHotspotUptime();

/**
 * Check if hotspot should auto-start based on conditions
 */
bool shouldAutoStartHotspot();

/**
 * Handle hotspot auto-management
 */
void handleHotspotAutoManagement();

/**
 * Get connection information string
 */
String getHotspotConnectionInfo();

/**
 * Update combined statistics
 */
void updateCombinedStatistics();

/**
 * Update BLE connection status
 */
void updateBLEConnectionStatus(bool connected, const String& client_info = "");

/**
 * Record BLE data transmission
 */
void recordBLEDataTransmission(uint32_t bytes_sent);

/**
 * Record BLE command received
 */
void recordBLECommandReceived();

/**
 * Get comprehensive connection info
 */
String getComprehensiveConnectionInfo();

/**
 * Should auto-start hotspot based on BLE status
 */
bool shouldAutoStartHotspotBasedOnBLE();

/**
 * Handle enhanced hotspot auto-management
 */
void handleEnhancedHotspotAutoManagement();

/**
 * Print comprehensive hotspot statistics
 */
void printComprehensiveHotspotStats();

#endif // HOTSPOT_MANAGER_H 