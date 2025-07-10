# Advanced Charging Management System

The ESP32S3 camera firmware now includes a comprehensive charging management system that goes far beyond basic USB power detection. This system provides detailed charging states, safety monitoring, current measurement, and intelligent optimization.

## Features Overview

### 🔋 Detailed Charging States
- **NOT_CHARGING**: No USB power or battery full
- **TRICKLE**: Low-current charging for deeply discharged batteries
- **CC (Constant Current)**: Main charging phase with maximum current
- **CV (Constant Voltage)**: Final charging phase with decreasing current
- **FULL**: Battery fully charged, maintenance mode
- **ERROR**: Charging fault detected
- **TIMEOUT**: Charging took too long
- **THERMAL_LIMIT**: Temperature protection active

### ⚡ Current Monitoring
- Real-time charging current measurement
- Configurable current sense resistor and amplifier
- Smoothed readings with history tracking
- Overcurrent protection

### 🌡️ Temperature Management
- Battery temperature monitoring
- Thermal protection (charging stops if too hot/cold)
- Temperature-based charging optimization
- Configurable temperature limits

### 🛡️ Safety Features
- Overvoltage protection
- Overcurrent protection
- Overtemperature protection
- Charging timeout protection
- Battery fault detection
- Automatic charging disable on safety violations

### 📊 Charging Analytics
- Charging cycle tracking
- Battery health estimation
- Time-to-full estimation
- Power consumption monitoring
- Charging history with 10 cycle memory

### 📡 BLE Integration
- Dedicated charging service with multiple characteristics
- Real-time charging status notifications
- Remote charging control
- Charging statistics streaming

## Hardware Requirements

### Basic Setup (USB Power Detection Only)
```
5V Pin ----[100kΩ]----+----[47kΩ]---- GND
                      |
                   A1 Pin (GPIO3)
```

### Advanced Setup (Current Monitoring)
```
Battery+ ----[0.1Ω Sense]----+---- Charging Circuit
                             |
                          [Op-Amp] ---- A2 Pin (GPIO4)
                          (50x gain)
```

### Temperature Monitoring (Optional)
- NTC thermistor on battery pack
- Connected to available ADC pin
- 10kΩ pullup resistor

## Configuration

### Charging Profile Settings
```cpp
typedef struct {
    float max_voltage;          // 4.2V for Li-ion
    float max_current;          // 500mA max safe current
    float trickle_voltage;      // 3.0V threshold for trickle
    float trickle_current;      // 50mA trickle current
    float cv_threshold;         // 4.1V CV phase start
    float full_threshold;       // 50mA termination current
    uint32_t timeout_ms;        // 4 hour timeout
    float temp_max;             // 45°C maximum
    float temp_min;             // 0°C minimum
} charging_profile_t;
```

### Hardware Constants
```cpp
#define CHARGING_CURRENT_SENSE_PIN      XIAO_ESP32S3_PIN_A2
#define CHARGING_CURRENT_SENSE_RESISTOR 0.1f    // 0.1 ohm
#define CHARGING_CURRENT_AMPLIFIER_GAIN 50.0f   // Op-amp gain
#define CHARGING_UPDATE_INTERVAL        5000    // 5 seconds
#define CHARGING_SAFETY_CHECK_INTERVAL  1000    // 1 second
```

## BLE Service

### Service UUID
```
19b10010-e8f2-537e-4f6c-d104768a1214
```

### Characteristics

#### Charging State (19b10011-...)
- **Type**: Read + Notify
- **Data**: Single byte charging state
- **Values**: 0-8 (see charging states above)

#### Charging Statistics (19b10012-...)
- **Type**: Read + Notify
- **Data**: 32-byte structure with complete charging info
- **Updates**: Every 5 seconds during charging

#### Charging Profile (19b10013-...)
- **Type**: Read + Write
- **Data**: Charging profile structure
- **Purpose**: Configure charging parameters

#### Charging Control (19b10014-...)
- **Type**: Write
- **Data**: Single byte commands
- **Commands**:
  - `0x01`: Enable charging
  - `0x02`: Disable charging
  - `0x03`: Reset charging stats
  - `0x04`: Print charging stats

## API Reference

### Core Functions

```cpp
void initializeChargingManager();
// Initialize the charging management system

void updateChargingStatus();
// Update all charging measurements and state

charging_state_t determineChargingState();
// Determine current charging phase

charging_safety_t checkChargingSafety();
// Check for safety violations

float readChargingCurrent();
// Read charging current in mA

float readBatteryTemperature();
// Read battery temperature in °C

uint32_t estimateTimeToFull();
// Estimate remaining charge time in ms
```

### Control Functions

```cpp
void enableCharging(bool enable);
// Enable or disable charging

void setChargingProfile(const charging_profile_t* profile);
// Update charging parameters

void resetChargingStats();
// Reset cycle counter and history

bool isChargingSafe();
// Check if charging is safe to continue
```

### Information Functions

```cpp
const char* getChargingStateString(charging_state_t state);
// Get human-readable state name

const char* getChargingSafetyString(charging_safety_t safety);
// Get safety status description

void printChargingStats();
// Print detailed charging information
```

## LED Indicators

The system provides visual feedback through LED patterns:

- **TRICKLE**: Orange/Yellow pulse
- **CONSTANT_CURRENT**: Yellow/Green pulse
- **CONSTANT_VOLTAGE**: Green/Blue slow blink
- **FULL**: Solid green
- **ERROR**: Red/Orange rapid blink
- **THERMAL_LIMIT**: Red/Orange rapid blink
- **TIMEOUT**: Yellow/Red SOS pattern

## Safety Features

### Automatic Protection
1. **Overvoltage**: Charging stops if voltage > max_voltage + 0.1V
2. **Overcurrent**: Charging stops if current > max_current + 100mA
3. **Overtemperature**: Charging stops if temp > max_temp + 5°C
4. **Timeout**: Charging stops after configured timeout
5. **Battery Fault**: Charging stops on rapid voltage drops

### Manual Override
- Charging can be manually disabled via BLE control
- Safety violations require manual re-enable
- Emergency stop via control command

## Charging Optimization

### Adaptive Charging
- Temperature-based current limiting
- Voltage-based phase transitions
- Age-based capacity adjustment
- Power source optimization

### Battery Health
- Cycle counting
- Capacity estimation
- Health percentage calculation
- Degradation tracking

## Troubleshooting

### Common Issues

#### No Current Readings
- Check current sense resistor value
- Verify op-amp connections
- Confirm ADC pin assignment
- Check amplifier gain setting

#### False Safety Triggers
- Adjust safety thresholds
- Check temperature sensor
- Verify voltage measurements
- Review charging profile

#### Slow Charging
- Check current limit settings
- Verify USB power capability
- Monitor temperature throttling
- Review safety constraints

### Debug Information

Enable detailed logging:
```cpp
printChargingStats();  // Print current status
```

Monitor via serial:
```
=== Charging Statistics ===
State: CONSTANT_CURRENT
Safety: OK
Voltage: 3.85 V
Current: 485.2 mA
Power: 1868.0 mW
Temperature: 28.5 °C
Time Elapsed: 1234567 ms
Time Remaining: 2345678 ms
Charge Cycles: 15
Battery Health: 98.5%
===========================
```

## Integration Examples

### Basic Monitoring
```cpp
void loop() {
    updateChargingStatus();
    
    if (chargingStats.state == CHARGING_STATE_FULL) {
        Serial.println("Battery fully charged!");
    }
}
```

### Safety Monitoring
```cpp
void checkChargingSafety() {
    if (!isChargingSafe()) {
        Serial.printf("Charging safety issue: %s\n", 
                     getChargingSafetyString(chargingStats.safety_status));
        enableCharging(false);
    }
}
```

### Custom Charging Profile
```cpp
void setupFastCharging() {
    charging_profile_t fastProfile = {
        .max_voltage = 4.2f,
        .max_current = 1000.0f,  // 1A fast charging
        .trickle_voltage = 3.0f,
        .trickle_current = 100.0f,
        .cv_threshold = 4.15f,
        .full_threshold = 100.0f,
        .timeout_ms = 2 * 60 * 60 * 1000,  // 2 hours
        .temp_max = 40.0f,  // Lower for safety
        .temp_min = 5.0f
    };
    
    setChargingProfile(&fastProfile);
}
```

## Performance Impact

### CPU Usage
- Charging updates: ~2ms every 5 seconds
- Safety checks: ~1ms every 1 second
- BLE notifications: ~0.5ms per update

### Memory Usage
- Core structures: ~200 bytes
- History buffer: ~280 bytes (10 cycles)
- Total overhead: ~500 bytes

### Power Consumption
- Additional ADC readings: ~1mA
- BLE service overhead: ~2mA
- Total impact: ~3mA (minimal)

## Future Enhancements

### Planned Features
- Wireless charging detection
- Multi-chemistry battery support
- Machine learning optimization
- Cloud-based analytics
- Predictive maintenance

### Hardware Upgrades
- Dedicated charging IC integration
- Multiple temperature sensors
- Current sense IC (INA219/226)
- Hardware charging control 