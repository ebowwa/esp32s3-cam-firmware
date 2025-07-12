#!/usr/bin/env python3
"""
Audio Timing Debug Tool
Analyzes timing issues in audio capture to understand data loss.
"""

import asyncio
import sys
import time
from bleak import BleakClient, BleakScanner

# BLE Configuration
DEVICE_NAME = "OpenGlass"
AUDIO_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

class AudioTimingDebugger:
    def __init__(self, duration=10):
        self.device = None
        self.duration = duration
        self.frames = []
        self.start_time = None
        self.last_frame_time = None
        self.frame_intervals = []
        
    async def find_device(self):
        """Find the OpenGlass device"""
        print("🔍 Scanning for device...")
        devices = await BleakScanner.discover(timeout=10.0)
        
        for device in devices:
            if device.name == DEVICE_NAME:
                self.device = device
                print(f"✅ Found: {device.name}")
                return True
        
        print("❌ Device not found!")
        return False
    
    def audio_callback(self, sender, data):
        """Handle incoming audio data with timing analysis"""
        current_time = time.time()
        
        if self.start_time is None:
            self.start_time = current_time
            self.last_frame_time = current_time
            print(f"🎤 Audio stream started at {current_time:.3f}")
        
        # Calculate frame interval
        if self.last_frame_time is not None:
            interval = current_time - self.last_frame_time
            self.frame_intervals.append(interval)
        
        # Store frame info
        frame_info = {
            'timestamp': current_time,
            'elapsed': current_time - self.start_time,
            'size': len(data),
            'interval': interval if self.last_frame_time else 0
        }
        self.frames.append(frame_info)
        
        # Show periodic updates
        if len(self.frames) % 20 == 0:
            elapsed = current_time - self.start_time
            expected_frames = elapsed * 19.2  # Expected ~19.2 fps
            actual_frames = len(self.frames)
            frame_rate = actual_frames / elapsed if elapsed > 0 else 0
            
            print(f"📊 {elapsed:.1f}s: {actual_frames} frames ({frame_rate:.1f} fps) - Expected: {expected_frames:.0f}")
            
            if len(self.frame_intervals) > 0:
                avg_interval = sum(self.frame_intervals[-20:]) / min(20, len(self.frame_intervals))
                print(f"   Avg interval: {avg_interval*1000:.1f}ms")
        
        self.last_frame_time = current_time
    
    async def debug_timing(self):
        """Debug audio timing"""
        if not self.device:
            return False
        
        try:
            print(f"🔗 Connecting to {self.device.address}...")
            async with BleakClient(self.device.address) as client:
                
                print(f"🎤 Starting {self.duration} second timing analysis...")
                await client.start_notify(AUDIO_CHARACTERISTIC_UUID, self.audio_callback)
                
                # Wait for the full duration
                await asyncio.sleep(self.duration)
                
                await client.stop_notify(AUDIO_CHARACTERISTIC_UUID)
                
                # Analyze results
                self.analyze_timing()
                return True
                
        except Exception as e:
            print(f"❌ Error: {e}")
            return False
    
    def analyze_timing(self):
        """Analyze the timing data"""
        if not self.frames:
            print("❌ No frames received!")
            return
        
        total_elapsed = self.frames[-1]['elapsed']
        total_frames = len(self.frames)
        actual_fps = total_frames / total_elapsed if total_elapsed > 0 else 0
        
        print(f"\n{'='*60}")
        print("TIMING ANALYSIS RESULTS")
        print(f"{'='*60}")
        
        print(f"📊 Overall Statistics:")
        print(f"   Requested duration: {self.duration} seconds")
        print(f"   Actual duration: {total_elapsed:.2f} seconds")
        print(f"   Total frames: {total_frames}")
        print(f"   Actual frame rate: {actual_fps:.2f} fps")
        print(f"   Expected frame rate: ~19.2 fps")
        print(f"   Expected frames: {self.duration * 19.2:.0f}")
        print(f"   Missing frames: {(self.duration * 19.2) - total_frames:.0f}")
        
        # Audio data analysis
        total_audio_bytes = sum(max(0, frame['size'] - 3) for frame in self.frames)  # Subtract header
        expected_audio_bytes = self.duration * 16000 * 2  # 16kHz * 2 bytes per sample
        
        print(f"\n📏 Audio Data Analysis:")
        print(f"   Total audio bytes received: {total_audio_bytes}")
        print(f"   Expected audio bytes: {expected_audio_bytes}")
        print(f"   Data completeness: {(total_audio_bytes/expected_audio_bytes)*100:.1f}%")
        print(f"   Audio duration: {total_audio_bytes/(16000*2):.2f} seconds")
        
        # Frame interval analysis
        if self.frame_intervals:
            import statistics
            avg_interval = statistics.mean(self.frame_intervals)
            median_interval = statistics.median(self.frame_intervals)
            min_interval = min(self.frame_intervals)
            max_interval = max(self.frame_intervals)
            
            print(f"\n⏱️  Frame Interval Analysis:")
            print(f"   Average interval: {avg_interval*1000:.1f}ms")
            print(f"   Median interval: {median_interval*1000:.1f}ms")
            print(f"   Min interval: {min_interval*1000:.1f}ms")
            print(f"   Max interval: {max_interval*1000:.1f}ms")
            print(f"   Expected interval: ~52ms (19.2 fps)")
            
            # Check for gaps
            large_gaps = [i for i in self.frame_intervals if i > 0.1]  # >100ms gaps
            if large_gaps:
                print(f"   ⚠️  Large gaps detected: {len(large_gaps)} intervals >100ms")
                print(f"      Largest gap: {max(large_gaps)*1000:.1f}ms")
        
        # Timeline analysis
        print(f"\n📈 Timeline Analysis:")
        print(f"   First frame: {self.frames[0]['elapsed']:.3f}s")
        print(f"   Last frame: {self.frames[-1]['elapsed']:.3f}s")
        
        # Check for streaming interruptions
        gaps = []
        for i in range(1, len(self.frames)):
            gap = self.frames[i]['elapsed'] - self.frames[i-1]['elapsed']
            if gap > 0.1:  # >100ms gap
                gaps.append((i, gap))
        
        if gaps:
            print(f"   ⚠️  Streaming interruptions detected: {len(gaps)} gaps >100ms")
            for i, gap in gaps[:5]:  # Show first 5 gaps
                print(f"      Gap at frame {i}: {gap*1000:.1f}ms")
        else:
            print(f"   ✅ No major streaming interruptions")
        
        # Diagnosis
        print(f"\n{'='*60}")
        print("DIAGNOSIS")
        print(f"{'='*60}")
        
        if total_audio_bytes < expected_audio_bytes * 0.5:
            print("❌ MAJOR DATA LOSS: Less than 50% of expected audio data")
            print("   Possible causes:")
            print("   - BLE connection issues")
            print("   - Audio capture not running continuously")
            print("   - Firmware timing issues")
            print("   - Buffer overflow in device")
        elif total_audio_bytes < expected_audio_bytes * 0.8:
            print("⚠️  MODERATE DATA LOSS: 50-80% of expected audio data")
            print("   Possible causes:")
            print("   - Intermittent BLE issues")
            print("   - Audio capture timing problems")
        else:
            print("✅ GOOD DATA CAPTURE: >80% of expected audio data")
            print("   Minor data loss is normal for BLE streaming")
        
        if len(gaps) > 0:
            print(f"\n🔧 Recommendations:")
            print(f"   - Check BLE connection stability")
            print(f"   - Verify audio capture cycle timing in firmware")
            print(f"   - Consider increasing BLE connection interval")
            print(f"   - Check for interference from other devices")

async def main():
    """Main function"""
    print("🔬 ESP32S3 Audio Timing Debugger")
    print("="*40)
    
    duration = 10
    if len(sys.argv) > 1:
        try:
            duration = int(sys.argv[1])
        except ValueError:
            print("Invalid duration, using default 10 seconds")
    
    debugger = AudioTimingDebugger(duration)
    
    if not await debugger.find_device():
        sys.exit(1)
    
    success = await debugger.debug_timing()
    
    if success:
        print("\n✅ Timing analysis completed")
    else:
        print("\n❌ Timing analysis failed")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n🛑 Analysis interrupted")
        sys.exit(1) 