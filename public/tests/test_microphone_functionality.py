#!/usr/bin/env python3
"""
ESP32S3 Microphone Functionality Test

This script tests the microphone functionality of the ESP32S3 camera firmware
by monitoring serial output and testing BLE audio data transmission.

Requirements:
    pip install pyserial bleak wave numpy

Usage:
    python test_microphone_functionality.py --port /dev/ttyUSB0
"""

import asyncio
import serial
import time
import struct
import wave
import numpy as np
import argparse
import json
import logging
from datetime import datetime
from typing import Optional, Dict, Any, List
import sys
import threading
from pathlib import Path

try:
    from bleak import BleakClient, BleakScanner
    from bleak.backends.characteristic import BleakGATTCharacteristic
except ImportError:
    print("Error: bleak library not installed. Run: pip install bleak")
    sys.exit(1)

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('microphone_test.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

# BLE Configuration
DEVICE_NAME = "OpenGlass"
SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
AUDIO_DATA_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"
AUDIO_CODEC_UUID = "19B10002-E8F2-537E-4F6C-D104768A1214"
DEVICE_STATUS_UUID = "19B10007-E8F2-537E-4F6C-D104768A1214"

# Audio Configuration (from firmware constants)
SAMPLE_RATE = 16000
SAMPLE_BITS = 16
FRAME_SIZE = 160
RECORDING_BUFFER_SIZE = 320  # FRAME_SIZE * 2
COMPRESSED_BUFFER_SIZE = 323  # RECORDING_BUFFER_SIZE + 3 header

# Test Configuration
TEST_DURATION = 30  # seconds
AUDIO_SAVE_DURATION = 10  # seconds of audio to save
MIN_AUDIO_FRAMES = 50  # minimum frames to consider test successful

class MicrophoneTest:
    def __init__(self, serial_port: str = None, baudrate: int = 921600):
        self.serial_port = serial_port
        self.baudrate = baudrate
        self.serial_connection = None
        self.ble_client: Optional[BleakClient] = None
        self.device_address: Optional[str] = None
        
        # Test data
        self.audio_frames: List[bytes] = []
        self.audio_frame_count = 0
        self.codec_id = None
        self.device_status = None
        self.serial_messages: List[str] = []
        
        # Test results
        self.test_results: Dict[str, Any] = {
            "timestamp": datetime.now().isoformat(),
            "tests_passed": 0,
            "tests_failed": 0,
            "test_details": []
        }
        
        # Threading
        self.serial_thread = None
        self.serial_running = False
        
    def record_test_result(self, test_name: str, passed: bool, details: str = ""):
        """Record a test result"""
        result = {
            "test_name": test_name,
            "passed": passed,
            "details": details,
            "timestamp": datetime.now().isoformat()
        }
        
        self.test_results["test_details"].append(result)
        
        if passed:
            self.test_results["tests_passed"] += 1
            logger.info(f"✅ {test_name}: PASSED - {details}")
        else:
            self.test_results["tests_failed"] += 1
            logger.error(f"❌ {test_name}: FAILED - {details}")
    
    def start_serial_monitoring(self):
        """Start monitoring serial output in a separate thread"""
        if not self.serial_port:
            return
            
        try:
            self.serial_connection = serial.Serial(self.serial_port, self.baudrate, timeout=1)
            self.serial_running = True
            self.serial_thread = threading.Thread(target=self._serial_monitor)
            self.serial_thread.daemon = True
            self.serial_thread.start()
            logger.info(f"Started serial monitoring on {self.serial_port}")
        except Exception as e:
            logger.error(f"Failed to start serial monitoring: {e}")
    
    def stop_serial_monitoring(self):
        """Stop serial monitoring"""
        self.serial_running = False
        if self.serial_thread:
            self.serial_thread.join(timeout=2)
        if self.serial_connection:
            self.serial_connection.close()
            
    def _serial_monitor(self):
        """Serial monitoring thread function"""
        while self.serial_running:
            try:
                if self.serial_connection.in_waiting > 0:
                    line = self.serial_connection.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.serial_messages.append(line)
                        # Log microphone-related messages
                        if any(keyword in line.lower() for keyword in ['microphone', 'i2s', 'audio', 'recording']):
                            logger.info(f"[SERIAL] {line}")
                time.sleep(0.01)
            except Exception as e:
                logger.error(f"Serial monitoring error: {e}")
                break
    
    async def scan_for_device(self) -> Optional[str]:
        """Scan for ESP32S3 device and return its address"""
        logger.info("Scanning for ESP32S3 device...")
        
        devices = await BleakScanner.discover(timeout=10.0)
        for device in devices:
            if device.name and DEVICE_NAME in device.name:
                logger.info(f"Found device: {device.name} ({device.address})")
                return device.address
                
        logger.error(f"Device '{DEVICE_NAME}' not found")
        return None
    
    async def connect_to_device(self) -> bool:
        """Connect to the ESP32S3 device"""
        if not self.device_address:
            self.device_address = await self.scan_for_device()
            if not self.device_address:
                return False
        
        try:
            self.ble_client = BleakClient(self.device_address)
            await self.ble_client.connect()
            logger.info(f"Connected to {self.device_address}")
            
            # Subscribe to audio data notifications
            await self.ble_client.start_notify(
                AUDIO_DATA_UUID, 
                self.audio_data_handler
            )
            
            # Subscribe to device status notifications
            await self.ble_client.start_notify(
                DEVICE_STATUS_UUID,
                self.device_status_handler
            )
            
            logger.info("Subscribed to audio data and device status notifications")
            return True
        except Exception as e:
            logger.error(f"Failed to connect: {e}")
            return False
    
    async def disconnect_from_device(self):
        """Disconnect from the ESP32S3 device"""
        if self.ble_client and self.ble_client.is_connected:
            await self.ble_client.disconnect()
            logger.info("Disconnected from device")
    
    def audio_data_handler(self, sender: BleakGATTCharacteristic, data: bytearray):
        """Handle audio data notifications"""
        if len(data) >= 3:
            # Parse frame header
            frame_count = struct.unpack('<H', data[0:2])[0]
            frame_type = data[2]
            audio_data = data[3:]
            
            self.audio_frames.append(bytes(audio_data))
            self.audio_frame_count += 1
            
            if self.audio_frame_count % 50 == 0:
                logger.info(f"Received {self.audio_frame_count} audio frames, latest size: {len(audio_data)} bytes")
    
    def device_status_handler(self, sender: BleakGATTCharacteristic, data: bytearray):
        """Handle device status notifications"""
        if len(data) >= 1:
            self.device_status = data[0]
            status_name = self.get_device_status_name(self.device_status)
            logger.info(f"Device Status: {status_name} (0x{self.device_status:02X})")
    
    def get_device_status_name(self, status: int) -> str:
        """Get device status name"""
        status_map = {
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
        return status_map.get(status, f"UNKNOWN_{status}")
    
    async def test_microphone_initialization(self) -> bool:
        """Test microphone initialization via serial output"""
        logger.info("Testing microphone initialization...")
        
        # Look for microphone initialization messages in serial output
        microphone_init_messages = [
            "microphone configured",
            "i2s.begin",
            "microphone_init",
            "audio buffer",
            "recording buffer"
        ]
        
        found_messages = []
        for message in self.serial_messages:
            for init_msg in microphone_init_messages:
                if init_msg.lower() in message.lower():
                    found_messages.append(message)
                    break
        
        if found_messages:
            details = f"Found {len(found_messages)} initialization messages"
            self.record_test_result("Microphone Initialization", True, details)
            return True
        else:
            self.record_test_result("Microphone Initialization", False, "No initialization messages found")
            return False
    
    async def test_audio_codec_detection(self) -> bool:
        """Test audio codec detection"""
        logger.info("Testing audio codec detection...")
        
        try:
            # Read codec characteristic
            codec_data = await self.ble_client.read_gatt_char(AUDIO_CODEC_UUID)
            if codec_data and len(codec_data) >= 1:
                self.codec_id = codec_data[0]
                codec_name = self.get_codec_name(self.codec_id)
                details = f"Codec ID: {self.codec_id} ({codec_name})"
                self.record_test_result("Audio Codec Detection", True, details)
                return True
            else:
                self.record_test_result("Audio Codec Detection", False, "No codec data received")
                return False
        except Exception as e:
            self.record_test_result("Audio Codec Detection", False, f"Error reading codec: {e}")
            return False
    
    def get_codec_name(self, codec_id: int) -> str:
        """Get codec name from ID"""
        codec_map = {
            1: "PCM 16kHz",
            11: "μ-law 8kHz",
            20: "Opus 16kHz"
        }
        return codec_map.get(codec_id, f"Unknown_{codec_id}")
    
    async def test_audio_data_transmission(self) -> bool:
        """Test audio data transmission"""
        logger.info(f"Testing audio data transmission for {TEST_DURATION} seconds...")
        
        initial_frame_count = self.audio_frame_count
        start_time = time.time()
        
        # Wait for audio data
        while time.time() - start_time < TEST_DURATION:
            await asyncio.sleep(0.1)
            
            # Show progress every 5 seconds
            if int(time.time() - start_time) % 5 == 0:
                frames_received = self.audio_frame_count - initial_frame_count
                logger.info(f"Progress: {frames_received} frames received in {int(time.time() - start_time)} seconds")
        
        frames_received = self.audio_frame_count - initial_frame_count
        
        if frames_received >= MIN_AUDIO_FRAMES:
            details = f"Received {frames_received} audio frames in {TEST_DURATION} seconds"
            self.record_test_result("Audio Data Transmission", True, details)
            return True
        else:
            details = f"Only received {frames_received} frames (minimum: {MIN_AUDIO_FRAMES})"
            self.record_test_result("Audio Data Transmission", False, details)
            return False
    
    async def test_audio_frame_structure(self) -> bool:
        """Test audio frame structure"""
        logger.info("Testing audio frame structure...")
        
        if not self.audio_frames:
            self.record_test_result("Audio Frame Structure", False, "No audio frames to analyze")
            return False
        
        # Analyze frame sizes
        frame_sizes = [len(frame) for frame in self.audio_frames[-10:]]  # Last 10 frames
        avg_frame_size = sum(frame_sizes) / len(frame_sizes)
        
        # Expected frame size depends on codec
        expected_size = RECORDING_BUFFER_SIZE // 2  # PCM 16-bit to 16-bit
        if self.codec_id == 11:  # μ-law
            expected_size = 200  # μ-law compressed size
        elif self.codec_id == 20:  # Opus
            expected_size = 100  # Opus compressed size (variable)
        
        if 50 <= avg_frame_size <= 500:  # Reasonable range
            details = f"Average frame size: {avg_frame_size:.1f} bytes (expected ~{expected_size})"
            self.record_test_result("Audio Frame Structure", True, details)
            return True
        else:
            details = f"Unexpected frame size: {avg_frame_size:.1f} bytes"
            self.record_test_result("Audio Frame Structure", False, details)
            return False
    
    async def test_i2s_configuration(self) -> bool:
        """Test I2S configuration via serial output"""
        logger.info("Testing I2S configuration...")
        
        # Look for I2S configuration messages
        i2s_messages = [
            "i2s.setallpins",
            "i2s.begin",
            "i2s_read",
            "pdm_mono_mode",
            "failed to initialize i2s"
        ]
        
        found_messages = []
        error_messages = []
        
        for message in self.serial_messages:
            message_lower = message.lower()
            for i2s_msg in i2s_messages:
                if i2s_msg.lower() in message_lower:
                    found_messages.append(message)
                    if "failed" in message_lower or "error" in message_lower:
                        error_messages.append(message)
                    break
        
        if error_messages:
            details = f"I2S errors found: {len(error_messages)}"
            self.record_test_result("I2S Configuration", False, details)
            return False
        elif found_messages:
            details = f"Found {len(found_messages)} I2S configuration messages"
            self.record_test_result("I2S Configuration", True, details)
            return True
        else:
            self.record_test_result("I2S Configuration", False, "No I2S messages found")
            return False
    
    def save_audio_data(self, filename: str = "recorded_audio.wav"):
        """Save received audio data to WAV file"""
        if not self.audio_frames:
            logger.warning("No audio data to save")
            return
        
        try:
            # Limit to AUDIO_SAVE_DURATION seconds of audio
            max_frames = int(SAMPLE_RATE / FRAME_SIZE * AUDIO_SAVE_DURATION)
            frames_to_save = self.audio_frames[:max_frames]
            
            # Convert audio frames to numpy array
            audio_data = b''.join(frames_to_save)
            
            # Handle different codecs
            if self.codec_id == 1:  # PCM
                # Convert bytes to 16-bit samples
                samples = np.frombuffer(audio_data, dtype=np.int16)
            else:
                # For compressed codecs, we can't directly convert to WAV
                # Save as raw data instead
                filename = filename.replace('.wav', '.raw')
                with open(filename, 'wb') as f:
                    f.write(audio_data)
                logger.info(f"Saved {len(audio_data)} bytes of raw audio data to {filename}")
                return
            
            # Save as WAV file
            with wave.open(filename, 'wb') as wav_file:
                wav_file.setnchannels(1)  # Mono
                wav_file.setsampwidth(2)  # 16-bit
                wav_file.setframerate(SAMPLE_RATE)
                wav_file.writeframes(samples.tobytes())
            
            duration = len(samples) / SAMPLE_RATE
            logger.info(f"Saved {duration:.1f} seconds of audio to {filename}")
            
        except Exception as e:
            logger.error(f"Failed to save audio data: {e}")
    
    def analyze_audio_quality(self) -> Dict[str, Any]:
        """Analyze audio quality metrics"""
        if not self.audio_frames:
            return {}
        
        try:
            # Combine recent frames for analysis
            recent_frames = self.audio_frames[-100:]  # Last 100 frames
            audio_data = b''.join(recent_frames)
            
            if self.codec_id == 1:  # PCM
                samples = np.frombuffer(audio_data, dtype=np.int16)
                
                # Calculate basic metrics
                rms = np.sqrt(np.mean(samples.astype(np.float32) ** 2))
                peak = np.max(np.abs(samples))
                zero_crossings = np.sum(np.diff(np.signbit(samples)))
                
                return {
                    "rms_level": float(rms),
                    "peak_level": int(peak),
                    "zero_crossings": int(zero_crossings),
                    "dynamic_range": float(peak - rms) if peak > 0 else 0,
                    "samples_analyzed": len(samples)
                }
            else:
                return {
                    "compressed_data_size": len(audio_data),
                    "compression_ratio": len(audio_data) / (len(recent_frames) * RECORDING_BUFFER_SIZE)
                }
                
        except Exception as e:
            logger.error(f"Audio quality analysis failed: {e}")
            return {}
    
    def print_test_results(self):
        """Print comprehensive test results"""
        print("\n" + "=" * 60)
        print("🎤 MICROPHONE TEST RESULTS")
        print("=" * 60)
        
        total_tests = len(self.test_results["test_details"])
        passed_tests = self.test_results["tests_passed"]
        failed_tests = self.test_results["tests_failed"]
        
        print(f"Total Tests: {total_tests}")
        print(f"Passed: {passed_tests}")
        print(f"Failed: {failed_tests}")
        print(f"Success Rate: {(passed_tests/total_tests*100):.1f}%" if total_tests > 0 else "0%")
        
        print("\nDetailed Results:")
        for result in self.test_results["test_details"]:
            status = "✅" if result["passed"] else "❌"
            print(f"{status} {result['test_name']}: {result['details']}")
        
        # Audio statistics
        print(f"\nAudio Statistics:")
        print(f"Total frames received: {self.audio_frame_count}")
        print(f"Codec ID: {self.codec_id} ({self.get_codec_name(self.codec_id) if self.codec_id else 'Unknown'})")
        
        # Audio quality analysis
        quality_metrics = self.analyze_audio_quality()
        if quality_metrics:
            print(f"Audio Quality Metrics:")
            for key, value in quality_metrics.items():
                print(f"  {key}: {value}")
        
        print(f"Serial messages captured: {len(self.serial_messages)}")
        
    def save_test_results(self, filename: str = "microphone_test_results.json"):
        """Save test results to JSON file"""
        results = {
            **self.test_results,
            "audio_statistics": {
                "total_frames": self.audio_frame_count,
                "codec_id": self.codec_id,
                "codec_name": self.get_codec_name(self.codec_id) if self.codec_id else None,
                "quality_metrics": self.analyze_audio_quality()
            },
            "serial_messages_count": len(self.serial_messages)
        }
        
        with open(filename, 'w') as f:
            json.dump(results, f, indent=2)
        
        logger.info(f"Test results saved to {filename}")
    
    async def run_all_tests(self):
        """Run all microphone tests"""
        logger.info("Starting ESP32S3 Microphone Test Suite")
        logger.info("=" * 50)
        
        # Start serial monitoring if port provided
        if self.serial_port:
            self.start_serial_monitoring()
            await asyncio.sleep(2)  # Give serial monitoring time to start
        
        try:
            # Connect to BLE device
            if not await self.connect_to_device():
                logger.error("Failed to connect to device. Aborting tests.")
                return
            
            # Wait for device to be ready
            await asyncio.sleep(3)
            
            # Run tests
            await self.test_audio_codec_detection()
            await asyncio.sleep(1)
            
            await self.test_i2s_configuration()
            await asyncio.sleep(1)
            
            await self.test_microphone_initialization()
            await asyncio.sleep(1)
            
            await self.test_audio_data_transmission()
            await asyncio.sleep(1)
            
            await self.test_audio_frame_structure()
            
            # Save audio data
            self.save_audio_data()
            
        except Exception as e:
            logger.error(f"Test suite error: {e}")
        finally:
            await self.disconnect_from_device()
            self.stop_serial_monitoring()
        
        # Print and save results
        self.print_test_results()
        self.save_test_results()

def main():
    parser = argparse.ArgumentParser(description="ESP32S3 Microphone Functionality Test")
    parser.add_argument("--port", "-p", type=str, help="Serial port for monitoring (optional)")
    parser.add_argument("--baudrate", "-b", type=int, default=921600, help="Serial baud rate (default: 921600)")
    parser.add_argument("--duration", "-d", type=int, default=30, help="Test duration in seconds (default: 30)")
    parser.add_argument("--output", "-o", type=str, help="Output file for test results (JSON)")
    
    args = parser.parse_args()
    
    # Update test duration if specified
    global TEST_DURATION
    TEST_DURATION = args.duration
    
    # Create test instance
    test = MicrophoneTest(serial_port=args.port, baudrate=args.baudrate)
    
    # Run tests
    try:
        asyncio.run(test.run_all_tests())
    except KeyboardInterrupt:
        logger.info("Test interrupted by user")
    except Exception as e:
        logger.error(f"Test failed: {e}")
    
    # Save results to custom file if specified
    if args.output:
        test.save_test_results(args.output)

if __name__ == "__main__":
    main() 