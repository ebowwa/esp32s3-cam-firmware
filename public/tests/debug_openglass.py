#!/usr/bin/env python3
import asyncio
import serial
import threading
import time
from bleak import BleakClient, BleakScanner

# OpenGlass UUIDs
PHOTO_DATA_UUID = "19B10005-E8F2-537E-4F6C-D104768A1214"
PHOTO_CONTROL_UUID = "19B10006-E8F2-537E-4F6C-D104768A1214"

class SerialMonitor:
    def __init__(self, port='/dev/tty.usbmodem101', baudrate=921600):
        self.port = port
        self.baudrate = baudrate
        self.running = False
        
    def start(self):
        self.running = True
        self.thread = threading.Thread(target=self._monitor)
        self.thread.daemon = True
        self.thread.start()
        
    def stop(self):
        self.running = False
        if hasattr(self, 'thread'):
            self.thread.join(timeout=1)
            
    def _monitor(self):
        try:
            with serial.Serial(self.port, self.baudrate, timeout=1) as ser:
                print(f"[SERIAL] Monitoring {self.port} at {self.baudrate} baud")
                while self.running:
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            print(f"[SERIAL] {line}")
                    time.sleep(0.01)
        except Exception as e:
            print(f"[SERIAL] Error: {e}")

class OpenGlassDebugger:
    def __init__(self):
        self.client = None
        self.photo_data = bytearray()
        self.photo_frames = 0
        self.receiving_photo = False
        
    async def connect(self):
        print("[BLE] Scanning for OpenGlass...")
        devices = await BleakScanner.discover()
        
        openglass_device = None
        for device in devices:
            if device.name == "OpenGlass":
                openglass_device = device
                break
                
        if not openglass_device:
            print("[BLE] OpenGlass not found!")
            return False
            
        print(f"[BLE] Found OpenGlass at {openglass_device.address}")
        
        self.client = BleakClient(openglass_device.address)
        await self.client.connect()
        
        if self.client.is_connected:
            print("[BLE] Connected!")
            return True
        else:
            print("[BLE] Connection failed!")
            return False
    
    def photo_data_handler(self, sender, data):
        """Handle incoming photo data"""
        if len(data) < 2:
            print(f"[BLE] Invalid photo data: {len(data)} bytes")
            return
            
        # Check for end marker
        if data[0] == 0xFF and data[1] == 0xFF:
            print(f"[BLE] Photo complete! {len(self.photo_data)} bytes in {self.photo_frames} frames")
            self.receiving_photo = False
            
            if len(self.photo_data) > 0:
                filename = f"debug_photo_{int(time.time())}.jpg"
                with open(filename, "wb") as f:
                    f.write(self.photo_data)
                print(f"[BLE] Photo saved to {filename}")
            
            self.photo_data = bytearray()
            self.photo_frames = 0
            return
            
        # Extract frame data
        frame_number = data[0] | (data[1] << 8)
        frame_data = data[2:]
        
        print(f"[BLE] Frame {frame_number}: {len(frame_data)} bytes")
        
        if not self.receiving_photo:
            print("[BLE] Starting photo reception...")
            self.receiving_photo = True
            self.photo_data = bytearray()
            self.photo_frames = 0
            
        self.photo_data.extend(frame_data)
        self.photo_frames += 1
    
    async def setup_notifications(self):
        """Setup photo data notifications"""
        try:
            await self.client.start_notify(PHOTO_DATA_UUID, self.photo_data_handler)
            print("[BLE] Photo notifications enabled")
            return True
        except Exception as e:
            print(f"[BLE] Notification setup failed: {e}")
            return False
    
    async def trigger_photo(self):
        """Trigger photo capture"""
        try:
            # Send 255 (0xFF) which is -1 as signed byte
            await self.client.write_gatt_char(PHOTO_CONTROL_UUID, bytearray([255]))
            print("[BLE] Photo trigger sent (255/0xFF)")
            return True
        except Exception as e:
            print(f"[BLE] Photo trigger failed: {e}")
            return False
    
    async def disconnect(self):
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("[BLE] Disconnected")

async def main():
    # Start serial monitor
    serial_monitor = SerialMonitor()
    serial_monitor.start()
    
    # Wait a bit for serial to start
    await asyncio.sleep(1)
    
    debugger = OpenGlassDebugger()
    
    try:
        print("=== OpenGlass Debug Session ===")
        
        # Connect
        if not await debugger.connect():
            return
            
        # Setup notifications
        if not await debugger.setup_notifications():
            return
            
        # Wait a bit
        await asyncio.sleep(2)
        
        # Trigger photo
        print("[DEBUG] Triggering photo capture...")
        if not await debugger.trigger_photo():
            return
            
        # Wait for response
        print("[DEBUG] Waiting for photo data...")
        await asyncio.sleep(15)
        
        if not debugger.receiving_photo and len(debugger.photo_data) == 0:
            print("[DEBUG] No photo data received!")
        
    except Exception as e:
        print(f"[DEBUG] Error: {e}")
    finally:
        await debugger.disconnect()
        serial_monitor.stop()

if __name__ == "__main__":
    asyncio.run(main()) 