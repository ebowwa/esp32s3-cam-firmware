#!/usr/bin/env python3
"""
Test script for BLE Web Interface functionality
Connects to ESP32S3 camera via BLE and retrieves web interface information
"""

import asyncio
import json
import webbrowser
from bleak import BleakScanner, BleakClient

# BLE Service and Characteristic UUIDs
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
WEB_URL_UUID = "19B10022-E8F2-537E-4F6C-D104768A1214"
WEB_CONTENT_UUID = "19B10023-E8F2-537E-4F6C-D104768A1214"
DEVICE_STATUS_UUID = "19B10007-E8F2-537E-4F6C-D104768A1214"

# Device identification
DEVICE_NAME = "OpenGlass Camera"

class BLEWebInterface:
    def __init__(self):
        self.client = None
        self.device_address = None
        self.web_url = None
        self.web_content_info = None
        
    async def scan_for_device(self, timeout=10):
        """Scan for the OpenGlass Camera device"""
        print(f"Scanning for '{DEVICE_NAME}' device...")
        
        devices = await BleakScanner.discover(timeout=timeout)
        
        for device in devices:
            if device.name and DEVICE_NAME.lower() in device.name.lower():
                print(f"Found device: {device.name} ({device.address})")
                self.device_address = device.address
                return True
                
        print(f"Device '{DEVICE_NAME}' not found")
        return False
    
    async def connect(self):
        """Connect to the BLE device"""
        if not self.device_address:
            print("No device address available. Please scan first.")
            return False
            
        try:
            print(f"Connecting to {self.device_address}...")
            self.client = BleakClient(self.device_address)
            await self.client.connect()
            print("Connected successfully!")
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from the BLE device"""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("Disconnected from device")
    
    async def get_web_url(self):
        """Read the web interface URL from BLE"""
        try:
            url_data = await self.client.read_gatt_char(WEB_URL_UUID)
            self.web_url = url_data.decode('utf-8')
            print(f"Web URL: {self.web_url}")
            return self.web_url
        except Exception as e:
            print(f"Failed to read web URL: {e}")
            return None
    
    async def get_web_content_info(self):
        """Read the web content information from BLE"""
        try:
            content_data = await self.client.read_gatt_char(WEB_CONTENT_UUID)
            content_str = content_data.decode('utf-8')
            self.web_content_info = json.loads(content_str)
            print(f"Web Content Info: {self.web_content_info}")
            return self.web_content_info
        except Exception as e:
            print(f"Failed to read web content info: {e}")
            return None
    
    async def get_device_status(self):
        """Read the device status from BLE"""
        try:
            status_data = await self.client.read_gatt_char(DEVICE_STATUS_UUID)
            if len(status_data) > 0:
                status_value = status_data[0]
                print(f"Device Status: 0x{status_value:02X}")
                return status_value
            else:
                print("No device status data received")
                return None
        except Exception as e:
            print(f"Failed to read device status: {e}")
            return None
    
    async def monitor_web_url_changes(self, duration=30):
        """Monitor web URL changes via BLE notifications"""
        print(f"Monitoring web URL changes for {duration} seconds...")
        
        def notification_handler(sender, data):
            url = data.decode('utf-8')
            print(f"Web URL updated: {url}")
            self.web_url = url
        
        try:
            await self.client.start_notify(WEB_URL_UUID, notification_handler)
            await asyncio.sleep(duration)
            await self.client.stop_notify(WEB_URL_UUID)
        except Exception as e:
            print(f"Failed to monitor web URL changes: {e}")
    
    def open_web_interface(self):
        """Open the web interface in the default browser"""
        if self.web_url:
            print(f"Opening web interface: {self.web_url}")
            webbrowser.open(self.web_url)
        else:
            print("No web URL available. Please read the web URL first.")

async def main():
    """Main test function"""
    web_interface = BLEWebInterface()
    
    try:
        # Scan for device
        if not await web_interface.scan_for_device():
            print("Device not found. Make sure the ESP32S3 camera is powered on and advertising.")
            return
        
        # Connect to device
        if not await web_interface.connect():
            print("Failed to connect to device.")
            return
        
        # Get web interface information
        print("\n=== Reading Web Interface Information ===")
        await web_interface.get_web_url()
        await web_interface.get_web_content_info()
        await web_interface.get_device_status()
        
        # Ask user if they want to open the web interface
        if web_interface.web_url:
            response = input(f"\nOpen web interface ({web_interface.web_url}) in browser? (y/n): ")
            if response.lower() == 'y':
                web_interface.open_web_interface()
        
        # Ask user if they want to monitor URL changes
        response = input("\nMonitor web URL changes for 30 seconds? (y/n): ")
        if response.lower() == 'y':
            await web_interface.monitor_web_url_changes(30)
        
    except KeyboardInterrupt:
        print("\nTest interrupted by user")
    except Exception as e:
        print(f"Test failed: {e}")
    finally:
        await web_interface.disconnect()

if __name__ == "__main__":
    print("BLE Web Interface Test")
    print("=" * 50)
    asyncio.run(main()) 