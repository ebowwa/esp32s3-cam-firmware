#!/usr/bin/env python3
import asyncio
import struct
from bleak import BleakClient, BleakScanner

PHOTO_CONTROL_UUID = "19B10006-E8F2-537E-4F6C-D104768A1214"

async def test_photo_values():
    print("Scanning for OpenGlass...")
    devices = await BleakScanner.discover()
    
    openglass_device = None
    for device in devices:
        if device.name == "OpenGlass":
            openglass_device = device
            break
            
    if not openglass_device:
        print("OpenGlass not found!")
        return
        
    print(f"Found OpenGlass at {openglass_device.address}")
    
    client = BleakClient(openglass_device.address)
    await client.connect()
    
    if not client.is_connected:
        print("Failed to connect!")
        return
        
    print("Connected! Testing different photo control values...")
    
    # Test different ways to send -1
    test_values = [
        (255, "255 (0xFF as unsigned)"),
        (struct.pack('b', -1)[0], "struct.pack('b', -1)[0]"),
        (0xFF, "0xFF literal"),
        ((-1) & 0xFF, "(-1) & 0xFF"),
    ]
    
    for value, description in test_values:
        try:
            print(f"Testing {description}: {value}")
            await client.write_gatt_char(PHOTO_CONTROL_UUID, bytearray([value]))
            print(f"  Sent successfully: {value}")
            await asyncio.sleep(3)  # Wait a bit between tests
        except Exception as e:
            print(f"  Failed: {e}")
    
    await client.disconnect()
    print("Disconnected")

if __name__ == "__main__":
    asyncio.run(test_photo_values()) 