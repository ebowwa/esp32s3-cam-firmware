/*
 * Battery Voltage Calibration Sketch for XIAO ESP32S3
 * 
 * This sketch helps calibrate the battery voltage reading by testing different
 * voltage divider ratios and comparing with actual battery voltage.
 * 
 * Instructions:
 * 1. Connect a known battery voltage (measure with multimeter)
 * 2. Upload this sketch and open Serial Monitor at 115200 baud
 * 3. Compare the calculated voltages with your multimeter reading
 * 4. Use the ratio that gives the closest match in your main firmware
 */

#define BATTERY_ADC_PIN A0
#define BATTERY_ADC_RESOLUTION 4096
#define BATTERY_REFERENCE_VOLTAGE 3.3
#define BATTERY_CHECK_SAMPLES 20  // More samples for calibration

float readRawBatteryVoltage() {
    uint32_t adcSum = 0;
    for (int i = 0; i < BATTERY_CHECK_SAMPLES; i++) {
        adcSum += analogRead(BATTERY_ADC_PIN);
        delay(5);
    }
    
    float averageAdc = (float)adcSum / BATTERY_CHECK_SAMPLES;
    float adcVoltage = (averageAdc / BATTERY_ADC_RESOLUTION) * BATTERY_REFERENCE_VOLTAGE;
    
    return adcVoltage;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println("\n" + String("=").substring(0, 60));
    Serial.println("XIAO ESP32S3 Battery Voltage Calibration");
    Serial.println(String("=").substring(0, 60));
    Serial.println("This tool helps calibrate battery voltage readings");
    Serial.println("Measure your actual battery voltage with a multimeter");
    Serial.println("and compare with the calculated values below");
    Serial.println(String("=").substring(0, 60) + "\n");
    
    analogReadResolution(12);
    delay(1000);
}

void loop() {
    float adcVoltage = readRawBatteryVoltage();
    
    Serial.println(String("-").substring(0, 50));
    Serial.printf("ADC Voltage (direct): %.3fV\n", adcVoltage);
    Serial.println();
    
    // Test different voltage divider ratios
    float ratios[] = {1.0, 1.2, 1.5, 1.8, 2.0, 2.2, 2.5, 3.0};
    int numRatios = sizeof(ratios) / sizeof(ratios[0]);
    
    Serial.println("Possible battery voltages with different ratios:");
    for (int i = 0; i < numRatios; i++) {
        float batteryVoltage = adcVoltage * ratios[i];
        Serial.printf("Ratio %.1f: %.3fV", ratios[i], batteryVoltage);
        
        // Add helpful indicators
        if (batteryVoltage >= 3.0 && batteryVoltage <= 4.3) {
            Serial.print(" ✓ (Valid Li-ion range)");
        }
        if (batteryVoltage >= 3.7 && batteryVoltage <= 4.2) {
            Serial.print(" ⭐ (Typical Li-ion range)");
        }
        Serial.println();
    }
    
    Serial.println();
    Serial.println("Instructions:");
    Serial.println("1. Measure actual battery voltage with multimeter");
    Serial.println("2. Find the ratio that gives closest match");
    Serial.println("3. Update BATTERY_VOLTAGE_DIVIDER in constants.h");
    Serial.println(String("-").substring(0, 50));
    
    delay(5000);  // Update every 5 seconds
} 