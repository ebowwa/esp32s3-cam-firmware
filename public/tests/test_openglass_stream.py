#!/usr/bin/env python3
import asyncio
import sys
import time
from bleak import BleakClient, BleakScanner

# OpenGlass UUIDs
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
VIDEO_SERVICE_UUID = "19B10010-E8F2-537E-4F6C-D104768A1214"
VIDEO_DATA_UUID = "19B10008-E8F2-537E-4F6C-D104768A1214"
VIDEO_CONTROL_UUID = "19B10009-E8F2-537E-4F6C-D104768A1214"
VIDEO_STATUS_UUID = "19B1000A-E8F2-537E-4F6C-D104768A1214"

class OpenGlassStreamClient:
    def __init__(self):
        self.client = None
        self.video_data = bytearray()
        self.video_frames = 0
        self.receiving_video = False
        self.total_frames_received = 0
        self.streaming_active = False
        self.start_time = None
        
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
            
            # Get device info and check for video characteristics
            try:
                services = self.client.services
                print(f"Available services: {len(list(services))}")
                video_data_found = False
                video_control_found = False
                video_status_found = False
                
                for service in services:
                    print(f"  Service: {service.uuid}")
                    for char in service.characteristics:
                        print(f"    Characteristic: {char.uuid} - {char.properties}")
                        if char.uuid.lower() == VIDEO_DATA_UUID.lower():
                            video_data_found = True
                        elif char.uuid.lower() == VIDEO_CONTROL_UUID.lower():
                            video_control_found = True
                        elif char.uuid.lower() == VIDEO_STATUS_UUID.lower():
                            video_status_found = True
                
                print(f"\nVideo streaming support:")
                print(f"  Video Data: {'✓' if video_data_found else '✗'}")
                print(f"  Video Control: {'✓' if video_control_found else '✗'}")
                print(f"  Video Status: {'✓' if video_status_found else '✗'}")
                
                if not video_data_found:
                    print("⚠️  Video data characteristic missing - firmware may need updating")
                if not video_control_found:
                    print("⚠️  Video control characteristic missing - firmware may need updating")
                    
            except Exception as e:
                print(f"Error getting services: {e}")
                
            return True
        else:
            print("Failed to connect to OpenGlass!")
            return False
    
    def video_data_handler(self, sender, data):
        """Handle incoming video data"""
        if len(data) < 3:
            print(f"Invalid video data packet: {len(data)} bytes")
            return
            
        # Check for end marker
        if data[0] == 0xFF and data[1] == 0xFF:
            frame_type = data[2]
            if frame_type == 0x02:  # Streaming frame
                print(f"Video frame complete! Received {len(self.video_data)} bytes in {self.video_frames} frames")
                
                # Save video frame to file
                if len(self.video_data) > 0:
                    filename = f"video_frame_{self.total_frames_received:04d}.jpg"
                    with open(filename, "wb") as f:
                        f.write(self.video_data)
                    print(f"Video frame saved to {filename}")
                
                # Reset for next frame
                self.video_data = bytearray()
                self.video_frames = 0
                self.total_frames_received += 1
                self.receiving_video = False
                
                # Calculate FPS if streaming
                if self.start_time and self.total_frames_received > 0:
                    elapsed = time.time() - self.start_time
                    fps = self.total_frames_received / elapsed
                    print(f"Average FPS: {fps:.2f} ({self.total_frames_received} frames in {elapsed:.1f}s)")
            return
            
        # Extract frame number, frame type, and data
        frame_number = data[0] | (data[1] << 8)
        frame_type = data[2]
        frame_data = data[3:]
        
        frame_type_str = "streaming" if frame_type == 0x02 else "photo"
        print(f"Received {frame_type_str} frame {frame_number}: {len(frame_data)} bytes")
        
        if not self.receiving_video:
            print("Starting video frame reception...")
            self.receiving_video = True
            self.video_data = bytearray()
            self.video_frames = 0
            
        # Append frame data
        self.video_data.extend(frame_data)
        self.video_frames += 1
    
    def video_status_handler(self, sender, data):
        """Handle video status updates"""
        if len(data) >= 6:
            streaming = data[0]
            fps = data[1]
            frame_count = data[2] | (data[3] << 8)
            dropped_frames = data[4] | (data[5] << 8)
            
            status = "STREAMING" if streaming else "STOPPED"
            print(f"Video Status: {status} | FPS: {fps} | Frames: {frame_count} | Dropped: {dropped_frames}")
    
    async def setup_notifications(self):
        """Setup video data and status notifications"""
        try:
            await self.client.start_notify(VIDEO_DATA_UUID, self.video_data_handler)
            print("Video data notifications enabled")
            
            # Try to enable video status notifications (optional for backward compatibility)
            try:
                await self.client.start_notify(VIDEO_STATUS_UUID, self.video_status_handler)
                print("Video status notifications enabled")
            except Exception as e:
                print(f"Video status notifications not available: {e}")
                print("Continuing without status notifications...")
            
            return True
        except Exception as e:
            print(f"Failed to setup notifications: {e}")
            return False
    
    async def start_video_streaming(self, fps=5):
        """Start video streaming at specified FPS"""
        try:
            if fps < 1 or fps > 10:
                print(f"Invalid FPS: {fps}. Must be between 1 and 10.")
                return False
            
            # Check if video control characteristic exists
            try:
                await self.client.write_gatt_char(VIDEO_CONTROL_UUID, bytearray([fps]))
                print(f"Video streaming started at {fps} FPS")
                self.streaming_active = True
                self.start_time = time.time()
                self.total_frames_received = 0
                return True
            except Exception as e:
                print(f"Video control characteristic not found: {e}")
                print("This firmware may not support video streaming yet.")
                print("Please recompile and upload the firmware with video streaming support.")
                return False
        except Exception as e:
            print(f"Failed to start video streaming: {e}")
            return False
    
    async def stop_video_streaming(self):
        """Stop video streaming"""
        try:
            # Send 0 to stop streaming
            await self.client.write_gatt_char(VIDEO_CONTROL_UUID, bytearray([0]))
            print("Video streaming stopped")
            self.streaming_active = False
            return True
        except Exception as e:
            print(f"Failed to stop video streaming: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from device"""
        if self.client and self.client.is_connected:
            await self.client.disconnect()
            print("Disconnected from OpenGlass")

async def main():
    if len(sys.argv) > 1:
        try:
            fps = int(sys.argv[1])
        except ValueError:
            print("Invalid FPS value. Using default 5 FPS.")
            fps = 5
    else:
        fps = 5
    
    client = OpenGlassStreamClient()
    
    try:
        # Connect to device
        if not await client.scan_and_connect():
            return
        
        # Setup notifications
        if not await client.setup_notifications():
            return
        
        # Start video streaming
        if not await client.start_video_streaming(fps):
            return
        
        print(f"Video streaming started at {fps} FPS. Press Ctrl+C to stop...")
        
        # Stream for specified duration or until interrupted
        try:
            while client.streaming_active:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            print("\nStopping video streaming...")
            await client.stop_video_streaming()
            await asyncio.sleep(1)  # Give time for stop command to process
        
        # Final statistics
        if client.start_time and client.total_frames_received > 0:
            elapsed = time.time() - client.start_time
            fps_actual = client.total_frames_received / elapsed
            print(f"\nFinal Statistics:")
            print(f"  Total frames received: {client.total_frames_received}")
            print(f"  Duration: {elapsed:.1f} seconds")
            print(f"  Average FPS: {fps_actual:.2f}")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        await client.disconnect()

if __name__ == "__main__":
    print("OpenGlass Video Streaming Test")
    print("Usage: python test_openglass_stream.py [fps]")
    print("  fps: Frame rate (1-10), default is 5")
    print("Press Ctrl+C to stop streaming\n")
    
    asyncio.run(main()) 