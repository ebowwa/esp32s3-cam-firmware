#!/usr/bin/env python3
import asyncio
import sys
from bleak import BleakClient, BleakScanner

# OpenGlass UUIDs
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
PHOTO_DATA_UUID = "19B10005-E8F2-537E-4F6C-D104768A1214"
PHOTO_CONTROL_UUID = "19B10006-E8F2-537E-4F6C-D104768A1214"
DEVICE_STATUS_UUID = "19B10007-E8F2-537E-4F6C-D104768A1214"

class OpenGlassClient:
    def __init__(self):
        self.client = None
        self.photo_data = bytearray()
        self.photo_frames = 0
        self.receiving_photo = False
        self.photo_received = False  # Track if photo was successfully received
        self.device_status = 0
        
    async def scan_and_connect(self):
        print("Scanning for OpenGlass device...")
        devices = await BleakScanner.discover()
        
        openglass_device = None
        for device in devices:
            if device.name == "OpenGlass":
                openglass_device = device
                break
                
        if not openglass_device:
            print("OpenGlass device not found!")
            return False
            
        print(f"Found OpenGlass at {openglass_device.address}")
        
        # Connect to device
        self.client = BleakClient(openglass_device.address)
        await self.client.connect()
        
        if self.client.is_connected:
            print("Connected to OpenGlass!")
            
            # Get device info
            try:
                services = self.client.services
                print(f"Available services: {len(list(services))}")
                for service in services:
                    print(f"  Service: {service.uuid}")
                    for char in service.characteristics:
                        print(f"    Characteristic: {char.uuid} - {char.properties}")
            except Exception as e:
                print(f"Error getting services: {e}")
                
            return True
        else:
            print("Failed to connect to OpenGlass!")
            return False
    
    def device_status_handler(self, sender, data):
        """Handle device status updates"""
        if len(data) >= 1:
            self.device_status = data[0]
            status_names = {
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
            status_name = status_names.get(self.device_status, f"UNKNOWN_{self.device_status:02X}")
            print(f"Device Status: {status_name} (0x{self.device_status:02X})")
    
    def photo_data_handler(self, sender, data):
        """Handle incoming photo data"""
        if len(data) < 2:
            print(f"Invalid photo data packet: {len(data)} bytes")
            return
            
        # Check for end marker
        if data[0] == 0xFF and data[1] == 0xFF:
            print(f"Photo complete! Received {len(self.photo_data)} bytes in {self.photo_frames} frames")
            self.receiving_photo = False
            self.photo_received = True  # Mark photo as successfully received
            
            # Save photo to file
            if len(self.photo_data) > 0:
                with open(f"captured_photo_{self.photo_frames}.jpg", "wb") as f:
                    f.write(self.photo_data)
                print(f"Photo saved to captured_photo_{self.photo_frames}.jpg")
            
            # Reset for next photo
            self.photo_data = bytearray()
            self.photo_frames = 0
            return
            
        # Extract frame number and data
        frame_number = data[0] | (data[1] << 8)
        frame_type = data[2] if len(data) > 2 else 0
        frame_data = data[3:] if len(data) > 3 else data[2:]
        
        print(f"Received frame {frame_number} (type: 0x{frame_type:02X}): {len(frame_data)} bytes")
        
        if not self.receiving_photo:
            print("Starting photo reception...")
            self.receiving_photo = True
            self.photo_data = bytearray()
            self.photo_frames = 0
            
        # Append frame data
        self.photo_data.extend(frame_data)
        self.photo_frames += 1
    
    async def setup_notifications(self):
        """Setup photo data and device status notifications"""
        try:
            await self.client.start_notify(PHOTO_DATA_UUID, self.photo_data_handler)
            print("Photo data notifications enabled")
            
            await self.client.start_notify(DEVICE_STATUS_UUID, self.device_status_handler)
            print("Device status notifications enabled")
            
            # Read initial device status
            status_data = await self.client.read_gatt_char(DEVICE_STATUS_UUID)
            if status_data:
                self.device_status_handler(None, status_data)
            
            return True
        except Exception as e:
            print(f"Failed to setup notifications: {e}")
            return False
    
    async def trigger_single_photo(self):
        """Trigger a single photo capture"""
        try:
            # Check device status first
            if self.device_status != 0x03:  # Not READY
                print(f"Warning: Device not ready (status: 0x{self.device_status:02X})")
            
            # Send -1 (255 as unsigned byte) to trigger single photo
            # -1 as signed byte = 255 as unsigned byte = 0xFF
            await self.client.write_gatt_char(PHOTO_CONTROL_UUID, bytearray([255]))
            print("Single photo trigger sent (value: 255/0xFF)")
            return True
        except Exception as e:
            print(f"Failed to trigger photo: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from device"""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("Disconnected from OpenGlass")

async def main():
    client = OpenGlassClient()
    
    try:
        # Connect to device
        if not await client.scan_and_connect():
            return
        
        # Setup notifications
        if not await client.setup_notifications():
            return
        
        # Wait a moment for device status
        await asyncio.sleep(2)
        
        # Trigger single photo
        if not await client.trigger_single_photo():
            return
        
        # Wait for photo data
        print("Waiting for photo data...")
        timeout = 15  # Increased timeout
        for i in range(timeout):
            await asyncio.sleep(1)
            if client.photo_received:
                break
            if i % 5 == 4:  # Every 5 seconds
                print(f"Still waiting... ({i+1}/{timeout}s)")
        
        # Check if photo was received
        if not client.photo_received:
            if client.receiving_photo:
                print("Photo reception in progress but not completed!")
                print(f"Received {len(client.photo_data)} bytes so far in {client.photo_frames} frames")
            else:
                print("No photo data received!")
                print(f"Final device status: 0x{client.device_status:02X}")
                
                # Try reading device status one more time
                try:
                    status_data = await client.client.read_gatt_char(DEVICE_STATUS_UUID)
                    if status_data:
                        print(f"Current device status: 0x{status_data[0]:02X}")
                except Exception as e:
                    print(f"Failed to read device status: {e}")
        else:
            print("Photo capture completed successfully!")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        await client.disconnect()

if __name__ == "__main__":
    asyncio.run(main()) 