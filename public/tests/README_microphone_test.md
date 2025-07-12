# ESP32S3 Microphone Functionality Test

This Python script comprehensively tests the microphone functionality of the ESP32S3 camera firmware, including I2S configuration, audio data transmission, and BLE audio streaming.

## Features Tested

### 🎤 Core Microphone Functions
- **I2S Configuration** - Verifies I2S interface setup (GPIO41/42, 16kHz, 16-bit)
- **Microphone Initialization** - Tests microphone buffer allocation and setup
- **Audio Data Transmission** - Validates BLE audio streaming functionality
- **Audio Frame Structure** - Analyzes audio frame format and integrity
- **Audio Codec Detection** - Identifies active audio codec (PCM/μ-law/Opus)

### 🔗 BLE Audio Integration
- **Audio Data Streaming** - Real-time audio data via BLE notifications
- **Codec Identification** - Automatic codec detection and configuration
- **Frame Analysis** - Audio frame structure validation
- **Quality Metrics** - Audio signal quality assessment

### 📊 Serial Monitoring
- **I2S Debug Messages** - Monitors I2S initialization and configuration
- **Error Detection** - Identifies microphone setup failures
- **Performance Metrics** - Tracks audio processing performance

## Hardware Requirements

### ESP32S3 Configuration
- **XIAO ESP32S3 Sense** with microphone
- **I2S Microphone** connected to GPIO41 (SCK) and GPIO42 (WS)
- **Firmware** with microphone functionality enabled

### Test Environment
- **Python 3.7+** with required dependencies
- **Bluetooth** capability for BLE connection
- **Serial Port** access for debug monitoring (optional)

## Installation

### Install Dependencies
```bash
pip install -r requirements.txt
```

### Required Python Packages
```bash
pip install pyserial bleak numpy wave
```

## Usage

### Basic Test Run
```bash
python test_microphone_functionality.py
```

### With Serial Monitoring
```bash
python test_microphone_functionality.py --port /dev/ttyUSB0
```

### Custom Test Duration
```bash
python test_microphone_functionality.py --duration 60 --port /dev/ttyUSB0
```

### Save Results to File
```bash
python test_microphone_functionality.py --output mic_test_results.json
```

## Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--port` | Serial port for debug monitoring | None |
| `--baudrate` | Serial baud rate | 921600 |
| `--duration` | Test duration in seconds | 30 |
| `--output` | Output file for results (JSON) | microphone_test_results.json |

## Test Sequence

### 1. Device Connection
- Scans for "OpenGlass" BLE device
- Establishes BLE connection
- Subscribes to audio data notifications

### 2. Audio Codec Detection
- Reads audio codec characteristic
- Identifies active codec (PCM/μ-law/Opus)
- Validates codec configuration

### 3. I2S Configuration Test
- Monitors serial output for I2S messages
- Verifies pin configuration (GPIO41/42)
- Checks for initialization errors

### 4. Microphone Initialization
- Validates microphone setup messages
- Checks buffer allocation
- Confirms I2S interface activation

### 5. Audio Data Transmission
- Captures audio frames via BLE
- Analyzes frame rate and consistency
- Validates minimum frame count

### 6. Audio Frame Structure
- Examines frame header format
- Validates frame size consistency
- Checks audio data integrity

## Expected Output

```
2024-01-15 10:30:00 - INFO - Starting ESP32S3 Microphone Test Suite
2024-01-15 10:30:01 - INFO - Scanning for ESP32S3 device...
2024-01-15 10:30:02 - INFO - Found device: OpenGlass (XX:XX:XX:XX:XX:XX)
2024-01-15 10:30:03 - INFO - Connected to XX:XX:XX:XX:XX:XX
2024-01-15 10:30:04 - INFO - Subscribed to audio data and device status notifications
2024-01-15 10:30:05 - INFO - ✅ Audio Codec Detection: PASSED - Codec ID: 1 (PCM 16kHz)
2024-01-15 10:30:06 - INFO - ✅ I2S Configuration: PASSED - Found 3 I2S configuration messages
2024-01-15 10:30:07 - INFO - ✅ Microphone Initialization: PASSED - Found 2 initialization messages
2024-01-15 10:30:08 - INFO - Testing audio data transmission for 30 seconds...
2024-01-15 10:30:13 - INFO - Progress: 125 frames received in 5 seconds
2024-01-15 10:30:18 - INFO - Progress: 250 frames received in 10 seconds
2024-01-15 10:30:38 - INFO - ✅ Audio Data Transmission: PASSED - Received 750 audio frames in 30 seconds
2024-01-15 10:30:39 - INFO - ✅ Audio Frame Structure: PASSED - Average frame size: 160.0 bytes
2024-01-15 10:30:40 - INFO - Saved 10.0 seconds of audio to recorded_audio.wav
```

## Audio Configuration

### I2S Settings (from firmware)
- **Sample Rate**: 16,000 Hz
- **Sample Bits**: 16-bit
- **Channels**: 1 (Mono)
- **Mode**: PDM (Pulse Density Modulation)
- **Frame Size**: 160 samples
- **Buffer Size**: 320 bytes (160 samples × 2 bytes)

### Pin Configuration
- **WS Pin**: GPIO42 (Word Select)
- **SCK Pin**: GPIO41 (Serial Clock)
- **Data Pin**: Internal PDM processing

### Supported Codecs
- **PCM 16kHz** (Codec ID: 1) - Uncompressed 16-bit audio
- **μ-law 8kHz** (Codec ID: 11) - Compressed 8-bit audio
- **Opus 16kHz** (Codec ID: 20) - Advanced compression

## Output Files

### Audio Recording
- **recorded_audio.wav** - 10 seconds of captured audio (PCM format)
- **recorded_audio.raw** - Raw audio data (compressed codecs)

### Test Results
- **microphone_test_results.json** - Detailed test results and metrics
- **microphone_test.log** - Complete test execution log

### Test Results JSON Structure
```json
{
  "timestamp": "2024-01-15T10:30:00",
  "tests_passed": 5,
  "tests_failed": 0,
  "test_details": [
    {
      "test_name": "Audio Codec Detection",
      "passed": true,
      "details": "Codec ID: 1 (PCM 16kHz)",
      "timestamp": "2024-01-15T10:30:05"
    }
  ],
  "audio_statistics": {
    "total_frames": 750,
    "codec_id": 1,
    "codec_name": "PCM 16kHz",
    "quality_metrics": {
      "rms_level": 1250.5,
      "peak_level": 8192,
      "zero_crossings": 1500,
      "dynamic_range": 6941.5
    }
  }
}
```

## Audio Quality Metrics

### Signal Analysis
- **RMS Level** - Root Mean Square amplitude
- **Peak Level** - Maximum signal amplitude
- **Zero Crossings** - Signal polarity changes (indicates frequency content)
- **Dynamic Range** - Difference between peak and RMS levels

### Frame Analysis
- **Frame Rate** - Audio frames per second
- **Frame Size** - Bytes per audio frame
- **Compression Ratio** - Data reduction for compressed codecs

## Troubleshooting

### Common Issues

**Device Not Found**
- Ensure ESP32S3 is powered and advertising
- Check device name is "OpenGlass"
- Verify Bluetooth is enabled

**No Audio Data**
- Check microphone is properly connected
- Verify I2S pins (GPIO41/42) are correctly wired
- Ensure firmware has microphone functionality enabled

**Serial Connection Failed**
- Verify correct serial port
- Check baud rate (default: 921600)
- Ensure no other programs are using the port

**Audio Quality Issues**
- Check microphone placement and environment
- Verify power supply stability
- Look for electromagnetic interference

### Debug Messages

**I2S Initialization**
```
[SERIAL] I2S.setAllPins(-1, 42, 41, -1, -1)
[SERIAL] I2S.begin(PDM_MONO_MODE, 16000, 16)
[SERIAL] Microphone configured
```

**Audio Buffer Allocation**
```
[SERIAL] Allocating audio buffers...
[SERIAL] Recording buffer: 320 bytes
[SERIAL] Compressed buffer: 323 bytes
```

**Error Messages**
```
[SERIAL] Failed to initialize I2S!
[SERIAL] Failed to allocate audio buffers!
[SERIAL] Audio capture failed
```

## Performance Expectations

### Normal Operation
- **Frame Rate**: ~100 frames/second (16kHz ÷ 160 samples)
- **Data Rate**: ~32 KB/second (320 bytes × 100 fps)
- **BLE Throughput**: ~25 KB/second (after BLE overhead)

### Test Thresholds
- **Minimum Frames**: 50 frames for 30-second test
- **Frame Size Range**: 50-500 bytes (depends on codec)
- **Audio Quality**: RMS > 100, Peak > 1000 (for active audio)

## Integration with Firmware

### Microphone Functions (firmware.ino)
```cpp
void configure_microphone() {
  // I2S initialization with GPIO41/42
  I2S.setAllPins(-1, I2S_WS_PIN, I2S_SCK_PIN, -1, -1);
  I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS);
  
  // Buffer allocation
  s_recording_buffer = PS_CALLOC_TRACKED(RECORDING_BUFFER_SIZE, ...);
}

size_t read_microphone() {
  // Read audio data from I2S
  esp_i2s::i2s_read(I2S_NUM_0, s_recording_buffer, 
                    RECORDING_BUFFER_SIZE, &bytes_recorded, ...);
  return bytes_recorded;
}
```

### BLE Audio Transmission
```cpp
void transmitAudioData(uint8_t *audioBuffer, size_t bufferSize) {
  // Add frame header
  compressedFrame[0] = audioFrameCount & 0xFF;
  compressedFrame[1] = (audioFrameCount >> 8) & 0xFF;
  compressedFrame[2] = 0; // Frame type
  
  // Encode audio data
  prepareAudioFrame(compressedFrame, audioBuffer, bytesRecorded);
  
  // Transmit via BLE
  notifyAudioData(compressedFrame, totalSize);
}
```

## Advanced Usage

### Continuous Monitoring
```bash
# Run test in a loop
while true; do
    python test_microphone_functionality.py --port /dev/ttyUSB0 --duration 60
    sleep 10
done
```

### Automated Testing
```bash
# Run test with different configurations
for duration in 30 60 120; do
    python test_microphone_functionality.py --duration $duration --output "test_${duration}s.json"
done
```

### Audio Analysis
```python
# Load and analyze saved audio
import wave
import numpy as np

with wave.open('recorded_audio.wav', 'rb') as wav:
    frames = wav.readframes(-1)
    samples = np.frombuffer(frames, dtype=np.int16)
    
    # Perform FFT analysis
    fft = np.fft.fft(samples)
    frequencies = np.fft.fftfreq(len(samples), 1/16000)
```

This comprehensive test validates all aspects of the ESP32S3 microphone functionality, from low-level I2S configuration to high-level audio streaming via BLE. 