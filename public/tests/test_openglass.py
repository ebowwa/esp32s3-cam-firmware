#!/usr/bin/env python3
import asyncio
import sys
from bleak import BleakClient, BleakScanner
import time

# OpenGlass UUIDs
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
PHOTO_DATA_UUID = "19B10005-E8F2-537E-4F6C-D104768A1214"
PHOTO_CONTROL_UUID = "19B10006-E8F2-537E-4F6C-D104768A1214"

class OpenGlassClient:
    def __init__(self):
        self.client = None
        self.device_address = None
        self.photo_data = bytearray()
        self.photo_frames = 0
        self.receiving_photo = False
        self.photo_received = False
        
    async def scan_and_connect(self, max_retries=3):
        """Scan for and connect to OpenGlass device with retry logic"""
        for attempt in range(max_retries):
            try:
                print(f"Scanning for OpenGlass device (attempt {attempt + 1}/{max_retries})...")
                devices = await BleakScanner.discover(timeout=10.0)
                
                openglass_device = None
                for device in devices:
                    if device.name == "OpenGlass":
                        openglass_device = device
                        break
                        
                if not openglass_device:
                    print("OpenGlass device not found!")
                    if attempt < max_retries - 1:
                        print("Retrying in 2 seconds...")
                        await asyncio.sleep(2)
                        continue
                    return False
                    
                print(f"Found OpenGlass at {openglass_device.address}")
                self.device_address = openglass_device.address
                
                # Connect to device with timeout
                self.client = BleakClient(openglass_device.address, timeout=30.0)
                
                print("Connecting to device...")
                await self.client.connect()
                
                if self.client.is_connected:
                    print("✅ Connected to OpenGlass!")
                    return True
                else:
                    print("❌ Failed to connect to OpenGlass!")
                    
            except Exception as e:
                print(f"Connection attempt {attempt + 1} failed: {e}")
                if self.client and self.client.is_connected:
                    try:
                        await self.client.disconnect()
                    except:
                        pass
                        
            if attempt < max_retries - 1:
                print("Retrying in 3 seconds...")
                await asyncio.sleep(3)
                
        return False
    
    def check_connection(self):
        """Check if client is connected"""
        connected = self.client and self.client.is_connected
        print(f"🔍 Connection status: {'✅ Connected' if connected else '❌ Disconnected'}")
        return connected
    
    def photo_data_handler(self, sender, data):
        """Handle incoming photo data"""
        if len(data) < 2:
            print(f"Invalid photo data packet: {len(data)} bytes")
            return
            
        # Check for end marker
        if data[0] == 0xFF and data[1] == 0xFF:
            print(f"📸 Photo complete! Received {len(self.photo_data)} bytes in {self.photo_frames} frames")
            self.receiving_photo = False
            self.photo_received = True
            
            # Save photo to file
            if len(self.photo_data) > 0:
                timestamp = int(time.time())
                filename = f"captured_photo_{timestamp}_{len(self.photo_data)}.jpg"
                with open(filename, "wb") as f:
                    f.write(self.photo_data)
                print(f"💾 Photo saved to {filename}")
            
            # Reset for next photo
            self.photo_data = bytearray()
            self.photo_frames = 0
            return
            
        # Extract frame number and data
        frame_number = data[0] | (data[1] << 8)
        frame_type = data[2] if len(data) > 2 else 0
        frame_data = data[3:] if len(data) > 3 else data[2:]
        
        print(f"📦 Received frame {frame_number} (type: 0x{frame_type:02X}): {len(frame_data)} bytes")
        
        if not self.receiving_photo:
            print("📸 Starting photo reception...")
            self.receiving_photo = True
            self.photo_data = bytearray()
            self.photo_frames = 0
            
        # Append frame data
        self.photo_data.extend(frame_data)
        self.photo_frames += 1
    
    async def setup_photo_notifications(self):
        """Setup photo data notifications"""
        try:
            if not self.check_connection():
                return False
                
            print("🔔 Setting up photo data notifications...")
            await self.client.start_notify(PHOTO_DATA_UUID, self.photo_data_handler)
            print("✅ Photo data notifications enabled")
            
            # Check connection after notification setup
            if not self.check_connection():
                print("❌ Connection lost after setting up notifications!")
                return False
                
            return True
            
        except Exception as e:
            print(f"❌ Failed to setup notifications: {e}")
            self.check_connection()
            return False
    
    async def trigger_photo_capture(self):
        """Trigger a single photo capture"""
        try:
            if not self.check_connection():
                return False
                
            print("📸 Triggering photo capture...")
            
            # Send -1 (255 as unsigned byte) to trigger single photo
            await self.client.write_gatt_char(PHOTO_CONTROL_UUID, bytearray([255]))
            print("✅ Photo capture command sent")
            
            # Check connection after write
            if not self.check_connection():
                print("❌ Connection lost after photo trigger!")
                return False
                
            return True
            
        except Exception as e:
            print(f"❌ Failed to trigger photo: {e}")
            self.check_connection()
            return False
    
    async def wait_for_photo(self, timeout_seconds=30):
        """Wait for photo data to be received"""
        print(f"⏳ Waiting for photo data (timeout: {timeout_seconds}s)...")
        
        for i in range(timeout_seconds):
            await asyncio.sleep(1)
            
            if self.photo_received:
                return True
                
            if i % 5 == 4:  # Every 5 seconds
                print(f"⏳ Still waiting... ({i+1}/{timeout_seconds}s)")
                
                # Check if still connected
                if not self.check_connection():
                    print("❌ Device disconnected while waiting for photo!")
                    return False
                    
                # Show progress if receiving
                if self.receiving_photo:
                    print(f"📦 Progress: {len(self.photo_data)} bytes received in {self.photo_frames} frames")
        
        return False
    
    async def disconnect(self):
        """Disconnect from device"""
        if self.client and self.client.is_connected:
            try:
                await self.client.disconnect()
                print("🔌 Disconnected from OpenGlass")
            except Exception as e:
                print(f"❌ Error during disconnect: {e}")

async def main():
    client = OpenGlassClient()
    
    try:
        # Step 1: Connect to device
        print("🔍 STEP 1: Connecting to OpenGlass")
        if not await client.scan_and_connect(max_retries=3):
            print("❌ Failed to connect to OpenGlass device")
            return
        
        # Verify connection
        client.check_connection()
        
        # Step 2: Setup photo notifications
        print("\n🔍 STEP 2: Setting up photo notifications")
        if not await client.setup_photo_notifications():
            print("❌ Failed to setup photo notifications")
            return
        
        # Step 3: Wait for connection to stabilize
        print("\n⏳ Waiting for connection to stabilize...")
        await asyncio.sleep(3)
        
        # Verify connection is still stable
        if not client.check_connection():
            print("❌ Connection lost during stabilization period")
            return
        
        # Step 4: Trigger photo capture
        print("\n🔍 STEP 3: Triggering photo capture")
        if not await client.trigger_photo_capture():
            print("❌ Failed to trigger photo capture")
            return
        
        # Step 5: Wait for photo data
        print("\n🔍 STEP 4: Waiting for photo data")
        if await client.wait_for_photo(timeout_seconds=30):
            print("\n🎉 SUCCESS: Photo capture completed!")
            print(f"📊 Final stats: {len(client.photo_data)} bytes received in {client.photo_frames} frames")
        else:
            print("\n❌ FAILED: Photo capture did not complete")
            if client.receiving_photo:
                print(f"📊 Partial data: {len(client.photo_data)} bytes received in {client.photo_frames} frames")
            else:
                print("💡 Possible causes:")
                print("   - Device camera not ready")
                print("   - Camera initialization failed")
                print("   - Photo control command not processed")
                print("   - Device firmware issue")
        
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()
    finally:
        await client.disconnect()

if __name__ == "__main__":
    asyncio.run(main()) 