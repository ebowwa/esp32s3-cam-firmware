#!/usr/bin/env python3
"""
OpenGlass Camera Diagnostic Tool

This script helps diagnose camera initialization issues by monitoring
device status and providing troubleshooting guidance.
"""

import asyncio
import sys
from bleak import BleakClient, BleakScanner

# OpenGlass UUIDs
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
DEVICE_STATUS_UUID = "19B10007-E8F2-537E-4F6C-D104768A1214"

class CameraDiagnostic:
    def __init__(self):
        self.client = None
        self.device_status = 0
        self.status_history = []
        
    async def scan_and_connect(self):
        print("🔍 Scanning for OpenGlass device...")
        devices = await BleakScanner.discover()
        
        openglass_device = None
        for device in devices:
            if device.name == "OpenGlass":
                openglass_device = device
                break
                
        if not openglass_device:
            print("❌ OpenGlass device not found!")
            print("   → Make sure the device is powered on and in range")
            return False
            
        print(f"✅ Found OpenGlass at {openglass_device.address}")
        
        # Connect to device
        self.client = BleakClient(openglass_device.address)
        await self.client.connect()
        
        if self.client.is_connected:
            print("✅ Connected to OpenGlass!")
            return True
        else:
            print("❌ Failed to connect to OpenGlass!")
            return False
    
    def device_status_handler(self, sender, data):
        """Handle device status updates"""
        if len(data) >= 1:
            self.device_status = data[0]
            self.status_history.append(self.device_status)
            
            status_info = self.get_status_info(self.device_status)
            print(f"📊 Device Status: {status_info['name']} (0x{self.device_status:02X})")
            print(f"   {status_info['description']}")
            
            if status_info['troubleshooting']:
                print(f"   💡 {status_info['troubleshooting']}")
    
    def get_status_info(self, status):
        """Get detailed information about device status"""
        status_map = {
            0x01: {
                "name": "INITIALIZING",
                "description": "Device is starting up",
                "troubleshooting": "Normal startup phase - wait for completion"
            },
            0x02: {
                "name": "WARMING_UP", 
                "description": "Sensors are stabilizing",
                "troubleshooting": "Normal warmup phase - sensors need time to stabilize"
            },
            0x03: {
                "name": "READY",
                "description": "Device is ready for operation",
                "troubleshooting": "✅ All systems operational!"
            },
            0x04: {
                "name": "ERROR",
                "description": "Device encountered an initialization error",
                "troubleshooting": "❌ Check camera connections, PSRAM, and power supply"
            },
            0x05: {
                "name": "CAMERA_INIT",
                "description": "Camera initialization in progress",
                "troubleshooting": "Camera hardware is being configured"
            },
            0x06: {
                "name": "MICROPHONE_INIT",
                "description": "Microphone initialization in progress", 
                "troubleshooting": "I2S microphone is being configured"
            },
            0x07: {
                "name": "BLE_INIT",
                "description": "Bluetooth initialization in progress",
                "troubleshooting": "BLE services are being set up"
            },
            0x08: {
                "name": "BATTERY_NOT_DETECTED",
                "description": "No battery detected",
                "troubleshooting": "Connect a lithium battery or continue with USB power"
            },
            0x09: {
                "name": "CHARGING",
                "description": "Device is charging",
                "troubleshooting": "✅ Battery is charging normally"
            },
            0x0A: {
                "name": "BATTERY_UNSTABLE",
                "description": "Battery connection is unstable",
                "troubleshooting": "Check battery connections and contacts"
            }
        }
        
        return status_map.get(status, {
            "name": f"UNKNOWN_{status:02X}",
            "description": "Unknown status code",
            "troubleshooting": "Unknown status - check firmware version"
        })
    
    async def monitor_device(self):
        """Monitor device status and provide guidance"""
        try:
            await self.client.start_notify(DEVICE_STATUS_UUID, self.device_status_handler)
            print("📡 Device status monitoring enabled")
            
            # Read initial status
            status_data = await self.client.read_gatt_char(DEVICE_STATUS_UUID)
            if status_data:
                self.device_status_handler(None, status_data)
            
            print("\n🔄 Monitoring device status (Press Ctrl+C to stop)...")
            print("   Watching for status changes during initialization...")
            
            # Monitor for 30 seconds
            for i in range(30):
                await asyncio.sleep(1)
                if i % 5 == 4:  # Every 5 seconds
                    print(f"   ⏱️  Monitoring... {i+1}/30s")
                    
        except Exception as e:
            print(f"❌ Error during monitoring: {e}")
    
    def print_diagnosis(self):
        """Print diagnosis based on status history"""
        print("\n" + "="*50)
        print("📋 DIAGNOSIS SUMMARY")
        print("="*50)
        
        if not self.status_history:
            print("❌ No status data received")
            return
            
        final_status = self.status_history[-1]
        unique_statuses = list(set(self.status_history))
        
        print(f"Final Status: {self.get_status_info(final_status)['name']}")
        print(f"Status Progression: {' → '.join([f'0x{s:02X}' for s in unique_statuses])}")
        
        if final_status == 0x04:  # ERROR
            print("\n❌ CAMERA INITIALIZATION FAILED")
            print("Possible causes:")
            print("1. Camera module not connected properly")
            print("2. PSRAM not available or faulty")
            print("3. Pin configuration mismatch")
            print("4. Power supply insufficient")
            print("5. Camera sensor not compatible")
            print("\nTroubleshooting steps:")
            print("1. Check all camera ribbon cable connections")
            print("2. Verify XIAO ESP32S3 Sense model (not regular ESP32S3)")
            print("3. Try different USB cable/power source")
            print("4. Re-flash firmware with latest version")
            print("5. Check for hardware defects")
            
        elif final_status == 0x03:  # READY
            print("\n✅ CAMERA INITIALIZATION SUCCESSFUL")
            print("Device is ready for photo capture!")
            
        elif final_status == 0x08:  # BATTERY_NOT_DETECTED
            print("\n⚠️  BATTERY NOT DETECTED")
            print("Device will work on USB power but battery recommended")
            
        else:
            print(f"\n⚠️  DEVICE IN {self.get_status_info(final_status)['name']} STATE")
            print("Device may not be fully initialized")
    
    async def disconnect(self):
        """Disconnect from device"""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("🔌 Disconnected from OpenGlass")

async def main():
    diagnostic = CameraDiagnostic()
    
    try:
        # Connect to device
        if not await diagnostic.scan_and_connect():
            return
        
        # Monitor device status
        await diagnostic.monitor_device()
        
        # Print diagnosis
        diagnostic.print_diagnosis()
        
    except KeyboardInterrupt:
        print("\n⏹️  Monitoring stopped by user")
        diagnostic.print_diagnosis()
    except Exception as e:
        print(f"❌ Error: {e}")
    finally:
        await diagnostic.disconnect()

if __name__ == "__main__":
    print("OpenGlass Camera Diagnostic Tool")
    print("=" * 35)
    asyncio.run(main()) 