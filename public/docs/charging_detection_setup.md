# Charging Detection Hardware Setup for XIAO ESP32S3

This guide explains how to set up hardware for detecting USB power and charging status on the XIAO ESP32S3.

## Required Components

- XIAO ESP32S3 (Sense or regular)
- 2x Resistors: 100kΩ and 47kΩ (for voltage divider)
- Jumper wires
- Breadboard (optional)
- Multimeter (for testing)

## Circuit Diagram

```
5V Pin ----[100kΩ]----+----[47kΩ]---- GND
                      |
                   A1 Pin (GPIO2)
```

## Wiring Instructions

1. **Connect the voltage divider:**
   - Connect a 100kΩ resistor between the 5V pin and A1 pin (GPIO2)
   - Connect a 47kΩ resistor between A1 pin (GPIO2) and GND

2. **Verify connections:**
   - 5V pin → 100kΩ resistor → A1 pin
   - A1 pin → 47kΩ resistor → GND

## How It Works

### USB Power Detection
- When USB is connected: 5V appears on the 5V pin
- Voltage divider reduces this to: 5V × (47kΩ / (100kΩ + 47kΩ)) = ~1.6V
- The firmware reads this voltage on A1 pin
- If voltage > 2.5V (threshold), USB power is detected

### Charging Detection Logic
1. **USB Power Check:** First, check if USB power is present
2. **Battery Voltage Check:** If USB is present, check battery voltage
3. **Charging Status:** If battery voltage > 4.1V and USB present, device is charging

## Calibration

If the readings are inaccurate:

1. **Measure actual voltages:**
   - Use a multimeter to measure voltage at A1 pin when USB is connected
   - Should read approximately 1.6V with 5V USB input

2. **Adjust constants in firmware:**
   - Modify `USB_POWER_VOLTAGE_DIVIDER` in `constants.h`
   - If measured voltage is higher than expected, decrease the value
   - If measured voltage is lower than expected, increase the value

## Testing

1. **Upload the firmware** with charging detection enabled
2. **Open serial monitor** at 115200 baud
3. **Test scenarios:**
   - Connect USB cable → Should show "CHARGING" status
   - Disconnect USB cable → Should show "READY" or "BATTERY_NOT_DETECTED"
   - Monitor debug output for voltage readings

## Troubleshooting

### Problem: No USB power detected
- Check resistor values and connections
- Verify 5V pin has 5V when USB is connected
- Adjust `USB_POWER_THRESHOLD` if needed

### Problem: False charging detection
- Check battery voltage readings
- Adjust `CHARGING_VOLTAGE_THRESHOLD` if needed
- Ensure battery is properly connected

### Problem: Inconsistent readings
- Add capacitor (100nF) across A1 pin to GND for filtering
- Increase sampling count in firmware
- Check for loose connections

## Safety Notes

- Never exceed 3.3V on GPIO pins
- Double-check resistor values before connecting
- Use a multimeter to verify voltages before connecting to GPIO pins
- The voltage divider limits current to safe levels

## Alternative Methods

If you don't want to add external components:

1. **Software-only detection:** Monitor D+ and D- pins (GPIO12, GPIO13) for USB activity
2. **Built-in LED monitoring:** Some boards have charging indicator LEDs that can be monitored
3. **Power consumption analysis:** Monitor current draw patterns to infer charging state

## Firmware Integration

The charging detection is automatically integrated with:
- Device status reporting via BLE
- Battery level monitoring
- Power management decisions
- Client notifications

Status codes:
- `0x03` (READY): Normal operation
- `0x08` (BATTERY_NOT_DETECTED): No battery detected
- `0x09` (CHARGING): Device is charging via USB power 