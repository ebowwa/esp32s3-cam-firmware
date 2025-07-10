#!/usr/bin/env python3
"""
Battery Detection Test Script for OpenGlass
Tests the battery detection functionality via BLE device status characteristic
"""

import asyncio
import logging
from bleak import BleakClient, BleakScanner
import struct

# BLE UUIDs
DEVICE_STATUS_UUID = "19b10007-e8f2-537e-4f6c-d104768a1214"
BATTERY_LEVEL_UUID = "00002a19-0000-1000-8000-00805f9b34fb"

# Device Status Values
DEVICE_STATUS_NAMES = {
    0x01: "INITIALIZING",
    0x02: "WARMING_UP", 
    0x03: "READY",
    0x04: "ERROR",
    0x05: "CAMERA_INIT",
    0x06: "MICROPHONE_INIT",
    0x07: "BLE_INIT",
    0x08: "BATTERY_NOT_DETECTED",
    0x09: "CHARGING",
    0x0A: "BATTERY_UNSTABLE"
}

class BatteryDetectionTester:
    def __init__(self):
        self.client = None
        self.device_status = None
        self.battery_level = None
        
    async def find_openglass_device(self):
        """Find OpenGlass device by scanning for BLE devices"""
        print("Scanning for OpenGlass device...")
        devices = await BleakScanner.discover()
        
        for device in devices:
            if device.name and "OpenGlass" in device.name:
                print(f"Found OpenGlass device: {device.name} ({device.address})")
                return device.address
                
        print("OpenGlass device not found!")
        return None
        
    async def device_status_callback(self, sender, data):
        """Callback for device status notifications"""
        if len(data) >= 1:
            status_value = data[0]
            status_name = DEVICE_STATUS_NAMES.get(status_value, f"UNKNOWN_STATUS_{status_value}")
            print(f"Device Status: {status_name} (0x{status_value:02x})")
            
            # Provide specific feedback based on status
            if status_value == 0x03:  # READY
                print("✅ Device ready - battery detected and connected")
            elif status_value == 0x08:  # BATTERY_NOT_DETECTED
                print("⚠️  No battery detected - check battery connection")
            elif status_value == 0x09:  # CHARGING
                print("🔋 Device is charging via USB power")
            elif status_value == 0x04:  # ERROR
                print("❌ Device error - check hardware connections")
            elif status_value == 0x0A:  # BATTERY_UNSTABLE
                print("⚠️  Battery unstable - check battery connection or power source")
            else:
                print(f"ℹ️  Device status: {status_name}")
            
            self.device_status = status_value
            
    async def battery_level_callback(self, sender, data):
        """Callback for battery level notifications"""
        if len(data) >= 1:
            battery_level = data[0]
            print(f"Battery Level: {battery_level}%")
            self.battery_level = battery_level
            
    async def test_battery_detection(self):
        """Main test function"""
        # Find device
        device_address = await self.find_openglass_device()
        if not device_address:
            return False
            
        # Connect to device
        print(f"Connecting to {device_address}...")
        self.client = BleakClient(device_address)
        
        try:
            await self.client.connect()
            print("Connected successfully!")
            
            # Subscribe to device status notifications
            await self.client.start_notify(DEVICE_STATUS_UUID, self.device_status_callback)
            print("Subscribed to device status notifications")
            
            # Subscribe to battery level notifications
            await self.client.start_notify(BATTERY_LEVEL_UUID, self.battery_level_callback)
            print("Subscribed to battery level notifications")
            
            # Read initial values
            status_data = await self.client.read_gatt_char(DEVICE_STATUS_UUID)
            if status_data:
                await self.device_status_callback(None, status_data)
                
            battery_data = await self.client.read_gatt_char(BATTERY_LEVEL_UUID)
            if battery_data:
                await self.battery_level_callback(None, battery_data)
            
            print("==================================================")
            print("BATTERY CONNECTION MONITORING TEST")
            print("==================================================")
            print("Monitoring device status and battery connection quality...")
            print("Test scenarios:")
            print("1. Connect/disconnect the battery to test detection")
            print("2. Connect/disconnect USB power to test charging detection")
            print("3. Gently wiggle battery connector to test stability detection")
            print("4. Monitor connection quality and voltage stability")
            print("Press Ctrl+C to stop")
            print("==================================================")
            print()
            
            # Monitor for 60 seconds or until interrupted
            await asyncio.sleep(60)
            
        except KeyboardInterrupt:
            print("\nTest interrupted by user")
        except Exception as e:
            print(f"Error during test: {e}")
        finally:
            if self.client.is_connected:
                await self.client.disconnect()
                print("Disconnected from device")
                
        return True
        
    def print_test_summary(self):
        """Print test summary"""
        print("\n" + "="*50)
        print("TEST SUMMARY")
        print("="*50)
        
        if self.device_status is not None:
            status_name = DEVICE_STATUS_NAMES.get(self.device_status, f"UNKNOWN({self.device_status})")
            print(f"Final Device Status: {status_name}")
            
            if self.device_status == 0x08:
                print("❌ Battery NOT detected")
            elif self.device_status == 0x03:
                print("✅ Battery detected and device ready")
            elif self.device_status == 0x0A:
                print("⚠️  Battery unstable")
            else:
                print("⚠️  Device in intermediate state")
        else:
            print("❓ No device status received")
            
        if self.battery_level is not None:
            print(f"Battery Level: {self.battery_level}%")
            if self.battery_level == 0:
                print("❌ Battery level indicates no battery")
            else:
                print("✅ Battery level indicates battery present")
        else:
            print("❓ No battery level received")
            
        print("="*50)

async def main():
    """Main function"""
    logging.basicConfig(level=logging.INFO)
    
    tester = BatteryDetectionTester()
    success = await tester.test_battery_detection()
    
    if success:
        tester.print_test_summary()
    else:
        print("Test failed - could not connect to device")

if __name__ == "__main__":
    asyncio.run(main()) 