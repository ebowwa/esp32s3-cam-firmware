#!/usr/bin/env python3
"""
ESP32S3 Hotspot Functionality Test

This script tests the hotspot functionality of the ESP32S3 camera firmware
by sending BLE commands and monitoring responses.

Requirements:
    pip install bleak asyncio-mqtt pywifi

Usage:
    python test_hotspot_functionality.py
"""

import asyncio
import json
import time
import logging
from datetime import datetime
from typing import Optional, Dict, Any
import sys

try:
    from bleak import BleakClient, BleakScanner
    from bleak.backends.characteristic import BleakGATTCharacteristic
except ImportError:
    print("Error: bleak library not installed. Run: pip install bleak")
    sys.exit(1)

try:
    import pywifi
    from pywifi import const
except ImportError:
    print("Warning: pywifi library not installed. WiFi scanning disabled. Run: pip install pywifi")
    pywifi = None

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('hotspot_test.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

# BLE Configuration
DEVICE_NAME = "OpenGlass"
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
HOTSPOT_CONTROL_UUID = "19B1000B-E8F2-537E-4F6C-D104768A1214"
HOTSPOT_STATUS_UUID = "19B1000C-E8F2-537E-4F6C-D104768A1214"

# Hotspot Control Commands
HOTSPOT_STOP = 0x00
HOTSPOT_START = 0x01
HOTSPOT_TOGGLE = 0x02
HOTSPOT_GET_STATUS = 0x03

# Test Configuration
TEST_TIMEOUT = 30  # seconds
HOTSPOT_SSID_PREFIX = "ESP32CAM-"
HOTSPOT_PASSWORD = "openglass123"
EXPECTED_IP = "192.168.4.1"

class HotspotTest:
    def __init__(self):
        self.client: Optional[BleakClient] = None
        self.device_address: Optional[str] = None
        self.hotspot_status_data: Dict[str, Any] = {}
        self.status_notifications: list = []
        self.test_results: Dict[str, Any] = {
            "timestamp": datetime.now().isoformat(),
            "tests_passed": 0,
            "tests_failed": 0,
            "test_details": []
        }
        
    async def scan_for_device(self) -> Optional[str]:
        """Scan for ESP32S3 device and return its address"""
        logger.info("Scanning for ESP32S3 device...")
        
        devices = await BleakScanner.discover(timeout=10.0)
        for device in devices:
            if device.name and DEVICE_NAME in device.name:
                logger.info(f"Found device: {device.name} ({device.address})")
                return device.address
                
        logger.error(f"Device '{DEVICE_NAME}' not found")
        return None
    
    async def connect_to_device(self) -> bool:
        """Connect to the ESP32S3 device"""
        if not self.device_address:
            self.device_address = await self.scan_for_device()
            if not self.device_address:
                return False
        
        try:
            self.client = BleakClient(self.device_address)
            await self.client.connect()
            logger.info(f"Connected to {self.device_address}")
            
            # Subscribe to status notifications
            await self.client.start_notify(
                HOTSPOT_STATUS_UUID, 
                self.status_notification_handler
            )
            logger.info("Subscribed to hotspot status notifications")
            
            return True
        except Exception as e:
            logger.error(f"Failed to connect: {e}")
            return False
    
    async def disconnect_from_device(self):
        """Disconnect from the ESP32S3 device"""
        if self.client and self.client.is_connected:
            try:
                await self.client.stop_notify(HOTSPOT_STATUS_UUID)
                await self.client.disconnect()
                logger.info("Disconnected from device")
            except Exception as e:
                logger.error(f"Error during disconnect: {e}")
    
    def status_notification_handler(self, sender: BleakGATTCharacteristic, data: bytearray):
        """Handle hotspot status notifications"""
        try:
            # Parse enhanced status data with BLE integration
            pos = 0
            status = {
                "timestamp": datetime.now().isoformat(),
                "raw_data": data.hex()
            }
            
            if len(data) < 5:
                logger.warning(f"Status data too short: {len(data)} bytes")
                return
            
            # Basic hotspot status
            status["status_code"] = data[pos]
            pos += 1
            status["connected_clients"] = data[pos]
            pos += 1
            status["max_clients"] = data[pos]
            pos += 1
            
            # BLE integration data
            if len(data) > pos:
                status["ble_connected"] = data[pos] == 1
                pos += 1
                
            if len(data) > pos:
                status["active_interfaces"] = data[pos]
                pos += 1
                
            # Data statistics (4 bytes each, little endian)
            if len(data) >= pos + 8:
                total_data = (data[pos] | (data[pos+1] << 8) | 
                             (data[pos+2] << 16) | (data[pos+3] << 24))
                status["total_data_bytes"] = total_data
                pos += 4
                
                ble_data = (data[pos] | (data[pos+1] << 8) | 
                           (data[pos+2] << 16) | (data[pos+3] << 24))
                status["ble_data_bytes"] = ble_data
                pos += 4
            
            # Parse SSID
            if len(data) > pos:
                ssid_len = data[pos]
                pos += 1
                if ssid_len > 0 and len(data) >= pos + ssid_len:
                    ssid = data[pos:pos+ssid_len].decode('utf-8', errors='ignore')
                    status["ssid"] = ssid
                    pos += ssid_len
                    
            # Parse IP address
            if len(data) > pos:
                ip_len = data[pos]
                pos += 1
                if ip_len > 0 and len(data) >= pos + ip_len:
                    ip = data[pos:pos+ip_len].decode('utf-8', errors='ignore')
                    status["ip_address"] = ip
                    pos += ip_len
            
            self.status_notifications.append(status)
            self.hotspot_status_data = status
            
            logger.info(f"Enhanced status notification: {status}")
            
        except Exception as e:
            logger.error(f"Error parsing status notification: {e}")
    
    async def send_hotspot_command(self, command: int) -> bool:
        """Send a hotspot control command"""
        if not self.client or not self.client.is_connected:
            logger.error("Not connected to device")
            return False
        
        try:
            command_bytes = bytes([command])
            await self.client.write_gatt_char(HOTSPOT_CONTROL_UUID, command_bytes)
            logger.info(f"Sent hotspot command: 0x{command:02X}")
            return True
        except Exception as e:
            logger.error(f"Failed to send command: {e}")
            return False
    
    async def wait_for_status_update(self, timeout: int = 10) -> bool:
        """Wait for a status notification"""
        start_time = time.time()
        initial_count = len(self.status_notifications)
        
        while (time.time() - start_time) < timeout:
            if len(self.status_notifications) > initial_count:
                return True
            await asyncio.sleep(0.1)
        
        return False
    
    def scan_wifi_networks(self) -> Optional[list]:
        """Scan for WiFi networks to detect hotspot"""
        if not pywifi:
            logger.warning("pywifi not available, skipping WiFi scan")
            return None
        
        try:
            wifi = pywifi.PyWiFi()
            iface = wifi.interfaces()[0]
            
            logger.info("Scanning for WiFi networks...")
            iface.scan()
            time.sleep(3)  # Wait for scan to complete
            
            networks = []
            for profile in iface.scan_results():
                networks.append({
                    "ssid": profile.ssid,
                    "signal": profile.signal,
                    "auth": profile.auth
                })
            
            return networks
        except Exception as e:
            logger.error(f"WiFi scan failed: {e}")
            return None
    
    def find_hotspot_network(self, networks: list) -> Optional[Dict]:
        """Find the ESP32S3 hotspot in WiFi networks"""
        if not networks:
            return None
        
        for network in networks:
            if network["ssid"].startswith(HOTSPOT_SSID_PREFIX):
                return network
        
        return None
    
    def record_test_result(self, test_name: str, passed: bool, details: str = ""):
        """Record a test result"""
        result = {
            "test_name": test_name,
            "passed": passed,
            "details": details,
            "timestamp": datetime.now().isoformat()
        }
        
        self.test_results["test_details"].append(result)
        
        if passed:
            self.test_results["tests_passed"] += 1
            logger.info(f"✅ {test_name}: PASSED - {details}")
        else:
            self.test_results["tests_failed"] += 1
            logger.error(f"❌ {test_name}: FAILED - {details}")
    
    async def test_hotspot_start(self) -> bool:
        """Test starting the hotspot"""
        logger.info("Testing hotspot start...")
        
        # Send start command
        if not await self.send_hotspot_command(HOTSPOT_START):
            self.record_test_result("Hotspot Start Command", False, "Failed to send command")
            return False
        
        # Wait for status update
        if not await self.wait_for_status_update(15):
            self.record_test_result("Hotspot Start Response", False, "No status update received")
            return False
        
        # Check if hotspot is active
        if self.hotspot_status_data.get("status_code") == 2:  # HOTSPOT_ACTIVE
            self.record_test_result("Hotspot Start", True, f"Hotspot active with SSID: {self.hotspot_status_data.get('ssid', 'Unknown')}")
            return True
        else:
            self.record_test_result("Hotspot Start", False, f"Unexpected status: {self.hotspot_status_data.get('status_code')}")
            return False
    
    async def test_hotspot_status(self) -> bool:
        """Test getting hotspot status"""
        logger.info("Testing hotspot status request...")
        
        # Send status request
        if not await self.send_hotspot_command(HOTSPOT_GET_STATUS):
            self.record_test_result("Hotspot Status Command", False, "Failed to send command")
            return False
        
        # Wait for status update
        if not await self.wait_for_status_update(5):
            self.record_test_result("Hotspot Status Response", False, "No status update received")
            return False
        
        # Validate status data
        status = self.hotspot_status_data
        if "ssid" in status and "ip_address" in status:
            self.record_test_result("Hotspot Status", True, f"Status received: {status}")
            return True
        else:
            self.record_test_result("Hotspot Status", False, f"Incomplete status data: {status}")
            return False
    
    async def test_wifi_detection(self) -> bool:
        """Test WiFi network detection"""
        logger.info("Testing WiFi network detection...")
        
        networks = self.scan_wifi_networks()
        if networks is None:
            self.record_test_result("WiFi Scan", False, "WiFi scanning not available")
            return False
        
        hotspot_network = self.find_hotspot_network(networks)
        if hotspot_network:
            self.record_test_result("WiFi Detection", True, f"Hotspot detected: {hotspot_network['ssid']}")
            return True
        else:
            self.record_test_result("WiFi Detection", False, "Hotspot network not found in scan")
            return False
    
    async def test_hotspot_stop(self) -> bool:
        """Test stopping the hotspot"""
        logger.info("Testing hotspot stop...")
        
        # Send stop command
        if not await self.send_hotspot_command(HOTSPOT_STOP):
            self.record_test_result("Hotspot Stop Command", False, "Failed to send command")
            return False
        
        # Wait for status update
        if not await self.wait_for_status_update(10):
            self.record_test_result("Hotspot Stop Response", False, "No status update received")
            return False
        
        # Check if hotspot is disabled
        if self.hotspot_status_data.get("status_code") == 0:  # HOTSPOT_DISABLED
            self.record_test_result("Hotspot Stop", True, "Hotspot successfully stopped")
            return True
        else:
            self.record_test_result("Hotspot Stop", False, f"Unexpected status: {self.hotspot_status_data.get('status_code')}")
            return False
    
    async def test_hotspot_toggle(self) -> bool:
        """Test toggling the hotspot"""
        logger.info("Testing hotspot toggle...")
        
        # Get initial status
        await self.send_hotspot_command(HOTSPOT_GET_STATUS)
        await self.wait_for_status_update(5)
        initial_status = self.hotspot_status_data.get("status_code", 0)
        
        # Send toggle command
        if not await self.send_hotspot_command(HOTSPOT_TOGGLE):
            self.record_test_result("Hotspot Toggle Command", False, "Failed to send command")
            return False
        
        # Wait for status update
        if not await self.wait_for_status_update(10):
            self.record_test_result("Hotspot Toggle Response", False, "No status update received")
            return False
        
        # Check if status changed
        new_status = self.hotspot_status_data.get("status_code", 0)
        if new_status != initial_status:
            self.record_test_result("Hotspot Toggle", True, f"Status changed from {initial_status} to {new_status}")
            return True
        else:
            self.record_test_result("Hotspot Toggle", False, f"Status unchanged: {initial_status}")
            return False
    
    async def test_ble_integration(self) -> bool:
        """Test BLE data integration in hotspot status"""
        logger.info("Testing BLE integration...")
        
        # Request status to get BLE integration data
        if not await self.send_hotspot_command(HOTSPOT_GET_STATUS):
            self.record_test_result("BLE Integration Command", False, "Failed to send command")
            return False
        
        # Wait for status update
        if not await self.wait_for_status_update(5):
            self.record_test_result("BLE Integration Response", False, "No status update received")
            return False
        
        # Check BLE integration data
        status = self.hotspot_status_data
        
        # Verify BLE connection is detected
        if "ble_connected" not in status:
            self.record_test_result("BLE Integration", False, "BLE connection status not reported")
            return False
        
        if not status["ble_connected"]:
            self.record_test_result("BLE Integration", False, "BLE connection not detected (expected True)")
            return False
        
        # Check for data statistics
        has_data_stats = ("total_data_bytes" in status and "ble_data_bytes" in status)
        if not has_data_stats:
            self.record_test_result("BLE Integration", False, "Data statistics not included")
            return False
        
        # Check active interfaces
        if "active_interfaces" not in status:
            self.record_test_result("BLE Integration", False, "Active interfaces not reported")
            return False
        
        # BLE should be active (bit 0 set)
        if not (status["active_interfaces"] & 0x01):
            self.record_test_result("BLE Integration", False, "BLE interface not marked as active")
            return False
        
        details = f"BLE connected, {status['ble_data_bytes']} bytes transmitted, interfaces: 0x{status['active_interfaces']:02X}"
        self.record_test_result("BLE Integration", True, details)
        return True
    
    async def run_all_tests(self):
        """Run all hotspot tests"""
        logger.info("Starting ESP32S3 Hotspot Test Suite")
        logger.info("=" * 50)
        
        try:
            # Connect to device
            if not await self.connect_to_device():
                logger.error("Failed to connect to device. Aborting tests.")
                return
            
            # Wait for initial connection
            await asyncio.sleep(2)
            
            # Test sequence
            await self.test_ble_integration()
            await asyncio.sleep(2)
            
            await self.test_hotspot_start()
            await asyncio.sleep(3)
            
            await self.test_hotspot_status()
            await asyncio.sleep(2)
            
            await self.test_wifi_detection()
            await asyncio.sleep(2)
            
            await self.test_hotspot_toggle()
            await asyncio.sleep(3)
            
            await self.test_hotspot_stop()
            await asyncio.sleep(2)
            
        except Exception as e:
            logger.error(f"Test suite error: {e}")
        finally:
            await self.disconnect_from_device()
        
        # Print results
        self.print_test_results()
        self.save_test_results()
    
    def print_test_results(self):
        """Print test results summary"""
        logger.info("\n" + "=" * 50)
        logger.info("TEST RESULTS SUMMARY")
        logger.info("=" * 50)
        
        total_tests = self.test_results["tests_passed"] + self.test_results["tests_failed"]
        logger.info(f"Total Tests: {total_tests}")
        logger.info(f"Passed: {self.test_results['tests_passed']}")
        logger.info(f"Failed: {self.test_results['tests_failed']}")
        
        if total_tests > 0:
            success_rate = (self.test_results["tests_passed"] / total_tests) * 100
            logger.info(f"Success Rate: {success_rate:.1f}%")
        
        logger.info("\nDetailed Results:")
        for test in self.test_results["test_details"]:
            status = "✅ PASS" if test["passed"] else "❌ FAIL"
            logger.info(f"  {status}: {test['test_name']} - {test['details']}")
        
        logger.info("\nStatus Notifications Received:")
        for i, notification in enumerate(self.status_notifications):
            logger.info(f"  {i+1}. {notification}")
    
    def save_test_results(self):
        """Save test results to file"""
        filename = f"hotspot_test_results_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        try:
            with open(filename, 'w') as f:
                json.dump(self.test_results, f, indent=2)
            logger.info(f"Test results saved to {filename}")
        except Exception as e:
            logger.error(f"Failed to save test results: {e}")

async def main():
    """Main test function"""
    test = HotspotTest()
    await test.run_all_tests()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Test interrupted by user")
    except Exception as e:
        logger.error(f"Test failed with error: {e}") 