#!/usr/bin/env python3
"""
Test script to analyze I2S buffer size and audio data patterns
This will help determine if the issue is with I2S capture or BLE transmission
"""

import asyncio
import struct
import wave
import numpy as np
from bleak import BleakClient, BleakScanner
import time
import sys

# BLE UUIDs
AUDIO_DATA_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"
AUDIO_CODEC_UUID = "19B10002-E8F2-537E-4F6C-D104768A1214"

class AudioAnalyzer:
    def __init__(self):
        self.frames = []
        self.frame_sizes = []
        self.frame_timestamps = []
        self.start_time = None
        self.total_bytes = 0
        self.frame_count = 0
        
    def analyze_frame(self, data):
        """Analyze a single audio frame"""
        if len(data) < 3:
            return
            
        # Extract frame header
        frame_count = struct.unpack('<H', data[0:2])[0]
        frame_type = data[2]
        audio_data = data[3:]
        
        current_time = time.time()
        if self.start_time is None:
            self.start_time = current_time
            
        elapsed = current_time - self.start_time
        
        # Store frame info
        self.frames.append({
            'frame_count': frame_count,
            'frame_type': frame_type,
            'data_size': len(audio_data),
            'timestamp': elapsed,
            'raw_data': audio_data
        })
        
        self.frame_sizes.append(len(audio_data))
        self.frame_timestamps.append(elapsed)
        self.total_bytes += len(audio_data)
        self.frame_count += 1
        
        # Print frame analysis
        print(f"Frame {frame_count}: {len(audio_data)} bytes, type={frame_type:02X}, time={elapsed:.3f}s")
        
        # Check for data patterns
        if len(audio_data) >= 8:
            # Show first 8 bytes
            hex_data = ' '.join(f'{b:02X}' for b in audio_data[:8])
            print(f"  First 8 bytes: {hex_data}")
            
            # Check if data is all zeros (silence)
            if all(b == 0 for b in audio_data):
                print(f"  WARNING: Frame {frame_count} contains all zeros (silence)")
            
            # Check for repeating patterns
            if len(audio_data) >= 16:
                first_8 = audio_data[:8]
                second_8 = audio_data[8:16]
                if first_8 == second_8:
                    print(f"  WARNING: Frame {frame_count} has repeating pattern")
    
    def print_summary(self):
        """Print analysis summary"""
        if not self.frames:
            print("No frames received!")
            return
            
        print("\n" + "="*50)
        print("AUDIO ANALYSIS SUMMARY")
        print("="*50)
        
        total_time = self.frame_timestamps[-1] if self.frame_timestamps else 0
        
        print(f"Total frames received: {self.frame_count}")
        print(f"Total audio data: {self.total_bytes} bytes")
        print(f"Recording duration: {total_time:.2f} seconds")
        print(f"Average frame size: {np.mean(self.frame_sizes):.1f} bytes")
        print(f"Frame size range: {min(self.frame_sizes)} - {max(self.frame_sizes)} bytes")
        print(f"Data rate: {self.total_bytes / total_time:.1f} bytes/second" if total_time > 0 else "Data rate: N/A")
        
        # Expected vs actual
        expected_bytes_per_second = 16000 * 2  # 16kHz * 16-bit
        expected_total = expected_bytes_per_second * total_time
        capture_efficiency = (self.total_bytes / expected_total * 100) if expected_total > 0 else 0
        
        print(f"Expected audio data: {expected_total:.0f} bytes")
        print(f"Capture efficiency: {capture_efficiency:.1f}%")
        
        # Frame timing analysis
        if len(self.frame_timestamps) > 1:
            intervals = np.diff(self.frame_timestamps)
            print(f"Average frame interval: {np.mean(intervals)*1000:.1f} ms")
            print(f"Frame interval range: {min(intervals)*1000:.1f} - {max(intervals)*1000:.1f} ms")
        
        # Check for missing frames
        frame_numbers = [f['frame_count'] for f in self.frames]
        if frame_numbers:
            expected_frames = list(range(frame_numbers[0], frame_numbers[-1] + 1))
            missing_frames = set(expected_frames) - set(frame_numbers)
            if missing_frames:
                print(f"Missing frames: {sorted(missing_frames)}")
            else:
                print("No missing frames detected")

async def test_audio_capture(duration_seconds=5):
    """Test audio capture and analyze the data"""
    print(f"🎤 I2S Buffer Size Test - {duration_seconds} seconds")
    print("="*50)
    
    # Find OpenGlass device
    print("Scanning for OpenGlass device...")
    devices = await BleakScanner.discover()
    openglass_device = None
    
    for device in devices:
        if device.name == "OpenGlass":
            openglass_device = device
            break
    
    if not openglass_device:
        print("❌ OpenGlass device not found!")
        return
    
    print(f"Found OpenGlass device: {openglass_device.address}")
    
    analyzer = AudioAnalyzer()
    
    def audio_callback(sender, data):
        """Handle incoming audio data"""
        analyzer.analyze_frame(data)
    
    # Connect and start audio capture
    async with BleakClient(openglass_device.address) as client:
        print("Connected to OpenGlass")
        
        # Start audio notifications
        await client.start_notify(AUDIO_DATA_UUID, audio_callback)
        print(f"🎤 Starting {duration_seconds} second audio analysis...")
        
        # Record for specified duration
        start_time = time.time()
        while time.time() - start_time < duration_seconds:
            await asyncio.sleep(0.1)
        
        # Stop notifications
        await client.stop_notify(AUDIO_DATA_UUID)
        print("Audio capture stopped")
    
    # Analyze results
    analyzer.print_summary()
    
    # Save raw data for further analysis
    if analyzer.frames:
        with open(f"audio_analysis_{int(time.time())}.txt", "w") as f:
            f.write("Frame Analysis Results\n")
            f.write("="*50 + "\n")
            for frame in analyzer.frames:
                f.write(f"Frame {frame['frame_count']}: {frame['data_size']} bytes at {frame['timestamp']:.3f}s\n")
        print("Detailed analysis saved to audio_analysis_*.txt")

if __name__ == "__main__":
    duration = int(sys.argv[1]) if len(sys.argv) > 1 else 5
    asyncio.run(test_audio_capture(duration)) 