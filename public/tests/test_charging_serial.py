#!/usr/bin/env python3
"""
ESP32S3 Charging System Test Script
Tests the modular charging manager via serial communication
"""

import serial
import time
import json
import argparse
import sys
from datetime import datetime
from typing import Dict, List, Optional

class ESP32ChargingTester:
    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.test_results = []
        
    def connect(self) -> bool:
        """Connect to ESP32 device"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=2)
            time.sleep(2)  # Wait for connection to stabilize
            print(f"✅ Connected to {self.port} at {self.baudrate} baud")
            return True
        except Exception as e:
            print(f"❌ Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from ESP32 device"""
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("🔌 Disconnected from device")
    
    def send_command(self, command: str) -> str:
        """Send command to ESP32 and read response"""
        if not self.ser or not self.ser.is_open:
            return "ERROR: Not connected"
        
        try:
            # Clear input buffer
            self.ser.reset_input_buffer()
            
            # Send command
            self.ser.write(f"{command}\n".encode())
            self.ser.flush()
            
            # Read response (multiple lines)
            response_lines = []
            start_time = time.time()
            
            while time.time() - start_time < 3:  # 3 second timeout
                if self.ser.in_waiting:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        response_lines.append(line)
                        print(f"📥 {line}")
                else:
                    time.sleep(0.1)
            
            return '\n'.join(response_lines)
            
        except Exception as e:
            return f"ERROR: {e}"
    
    def read_serial_output(self, duration: int = 10) -> List[str]:
        """Read serial output for specified duration"""
        print(f"📖 Reading serial output for {duration} seconds...")
        output_lines = []
        start_time = time.time()
        
        while time.time() - start_time < duration:
            if self.ser and self.ser.in_waiting:
                line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    output_lines.append(line)
                    print(f"📥 {line}")
            else:
                time.sleep(0.1)
        
        return output_lines
    
    def test_charging_initialization(self) -> Dict:
        """Test charging manager initialization"""
        print("\n🔋 Testing Charging Manager Initialization...")
        
        # Look for initialization messages
        output = self.read_serial_output(5)
        
        init_messages = [
            "Initializing Modular Charging Manager",
            "Charging Sensors initialized",
            "Charging State Machine initialized",
            "Charging Safety System initialized",
            "Charging History initialized",
            "Charging BLE Interface initialized"
        ]
        
        found_messages = []
        for line in output:
            for msg in init_messages:
                if msg in line:
                    found_messages.append(msg)
        
        result = {
            "test": "charging_initialization",
            "timestamp": datetime.now().isoformat(),
            "success": len(found_messages) >= 3,  # At least 3 components initialized
            "found_messages": found_messages,
            "total_output_lines": len(output)
        }
        
        self.test_results.append(result)
        print(f"{'✅' if result['success'] else '❌'} Initialization test: {len(found_messages)}/6 components found")
        return result
    
    def test_charging_status_updates(self) -> Dict:
        """Test charging status updates"""
        print("\n🔍 Testing Charging Status Updates...")
        
        # Read output and look for charging status messages
        output = self.read_serial_output(15)
        
        status_indicators = [
            "charging status update",
            "Charging state:",
            "Battery voltage:",
            "USB voltage:",
            "Charge Level:",
            "Safety:"
        ]
        
        found_indicators = []
        usb_events = []
        
        for line in output:
            for indicator in status_indicators:
                if indicator.lower() in line.lower():
                    found_indicators.append(line)
            
            # Look for USB connection events
            if "USB POWER CONNECTED" in line or "USB power disconnected" in line:
                usb_events.append(line)
        
        result = {
            "test": "charging_status_updates",
            "timestamp": datetime.now().isoformat(),
            "success": len(found_indicators) > 0,
            "status_updates": found_indicators,
            "usb_events": usb_events,
            "total_output_lines": len(output)
        }
        
        self.test_results.append(result)
        print(f"{'✅' if result['success'] else '❌'} Status updates test: {len(found_indicators)} updates found")
        return result
    
    def test_usb_connection_simulation(self) -> Dict:
        """Test USB connection detection"""
        print("\n🔌 Testing USB Connection Detection...")
        print("Please connect/disconnect USB power during this test...")
        
        # Monitor for USB connection events
        output = self.read_serial_output(20)
        
        usb_connected_events = []
        usb_disconnected_events = []
        voltage_readings = []
        
        for line in output:
            if "USB POWER CONNECTED" in line:
                usb_connected_events.append(line)
            elif "USB power disconnected" in line:
                usb_disconnected_events.append(line)
            elif "USB voltage detected:" in line:
                voltage_readings.append(line)
        
        result = {
            "test": "usb_connection_simulation",
            "timestamp": datetime.now().isoformat(),
            "success": len(usb_connected_events) > 0 or len(usb_disconnected_events) > 0,
            "connected_events": usb_connected_events,
            "disconnected_events": usb_disconnected_events,
            "voltage_readings": voltage_readings
        }
        
        self.test_results.append(result)
        print(f"{'✅' if result['success'] else '❌'} USB connection test: {len(usb_connected_events)} connect, {len(usb_disconnected_events)} disconnect events")
        return result
    
    def test_charging_safety_system(self) -> Dict:
        """Test charging safety system"""
        print("\n⚠️ Testing Charging Safety System...")
        
        # Look for safety-related messages
        output = self.read_serial_output(10)
        
        safety_messages = []
        safety_status = []
        
        for line in output:
            if "safety" in line.lower() or "Safety:" in line:
                safety_messages.append(line)
            if "CHARGING_SAFETY_" in line:
                safety_status.append(line)
        
        result = {
            "test": "charging_safety_system",
            "timestamp": datetime.now().isoformat(),
            "success": len(safety_messages) > 0,
            "safety_messages": safety_messages,
            "safety_status": safety_status
        }
        
        self.test_results.append(result)
        print(f"{'✅' if result['success'] else '❌'} Safety system test: {len(safety_messages)} safety messages found")
        return result
    
    def test_charging_state_machine(self) -> Dict:
        """Test charging state machine"""
        print("\n🔄 Testing Charging State Machine...")
        
        # Look for state transitions
        output = self.read_serial_output(15)
        
        state_transitions = []
        current_states = []
        
        for line in output:
            if "Charging state changed:" in line:
                state_transitions.append(line)
            elif "State:" in line and any(state in line for state in ["NOT_CHARGING", "TRICKLE", "CONSTANT_CURRENT", "CONSTANT_VOLTAGE", "FULL"]):
                current_states.append(line)
        
        result = {
            "test": "charging_state_machine",
            "timestamp": datetime.now().isoformat(),
            "success": len(state_transitions) > 0 or len(current_states) > 0,
            "state_transitions": state_transitions,
            "current_states": current_states
        }
        
        self.test_results.append(result)
        print(f"{'✅' if result['success'] else '❌'} State machine test: {len(state_transitions)} transitions, {len(current_states)} state reports")
        return result
    
    def test_sensor_readings(self) -> Dict:
        """Test sensor readings"""
        print("\n📊 Testing Sensor Readings...")
        
        # Look for sensor data
        output = self.read_serial_output(10)
        
        voltage_readings = []
        current_readings = []
        temperature_readings = []
        
        for line in output:
            if "Voltage:" in line:
                voltage_readings.append(line)
            elif "Current:" in line:
                current_readings.append(line)
            elif "Temperature:" in line:
                temperature_readings.append(line)
        
        result = {
            "test": "sensor_readings",
            "timestamp": datetime.now().isoformat(),
            "success": len(voltage_readings) > 0 or len(current_readings) > 0,
            "voltage_readings": voltage_readings,
            "current_readings": current_readings,
            "temperature_readings": temperature_readings
        }
        
        self.test_results.append(result)
        print(f"{'✅' if result['success'] else '❌'} Sensor readings test: {len(voltage_readings)} voltage, {len(current_readings)} current readings")
        return result
    
    def test_ble_integration(self) -> Dict:
        """Test BLE integration"""
        print("\n📡 Testing BLE Integration...")
        
        # Look for BLE-related messages
        output = self.read_serial_output(8)
        
        ble_messages = []
        ble_notifications = []
        
        for line in output:
            if "BLE" in line and "charging" in line.lower():
                ble_messages.append(line)
            elif "notification" in line.lower() or "characteristic" in line.lower():
                ble_notifications.append(line)
        
        result = {
            "test": "ble_integration",
            "timestamp": datetime.now().isoformat(),
            "success": len(ble_messages) > 0,
            "ble_messages": ble_messages,
            "ble_notifications": ble_notifications
        }
        
        self.test_results.append(result)
        print(f"{'✅' if result['success'] else '❌'} BLE integration test: {len(ble_messages)} BLE messages found")
        return result
    
    def run_comprehensive_test(self) -> Dict:
        """Run all charging tests"""
        print("🚀 Starting Comprehensive Charging System Test")
        print("=" * 60)
        
        if not self.connect():
            return {"error": "Failed to connect to device"}
        
        try:
            # Run all tests
            self.test_charging_initialization()
            self.test_charging_status_updates()
            self.test_usb_connection_simulation()
            self.test_charging_safety_system()
            self.test_charging_state_machine()
            self.test_sensor_readings()
            self.test_ble_integration()
            
            # Summary
            total_tests = len(self.test_results)
            passed_tests = sum(1 for result in self.test_results if result['success'])
            
            summary = {
                "total_tests": total_tests,
                "passed_tests": passed_tests,
                "failed_tests": total_tests - passed_tests,
                "success_rate": (passed_tests / total_tests) * 100 if total_tests > 0 else 0,
                "timestamp": datetime.now().isoformat(),
                "test_results": self.test_results
            }
            
            print("\n" + "=" * 60)
            print("📊 TEST SUMMARY")
            print("=" * 60)
            print(f"Total Tests: {total_tests}")
            print(f"Passed: {passed_tests}")
            print(f"Failed: {total_tests - passed_tests}")
            print(f"Success Rate: {summary['success_rate']:.1f}%")
            
            return summary
            
        finally:
            self.disconnect()
    
    def save_results(self, filename: str):
        """Save test results to JSON file"""
        with open(filename, 'w') as f:
            json.dump(self.test_results, f, indent=2)
        print(f"📄 Test results saved to {filename}")

def main():
    parser = argparse.ArgumentParser(description='ESP32S3 Charging System Tester')
    parser.add_argument('port', nargs='?', help='Serial port (e.g., /dev/ttyUSB0, COM3)')
    parser.add_argument('--port', '-p', dest='port_flag', help='Serial port (alternative flag format)')
    parser.add_argument('--baudrate', '-b', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--output', '-o', help='Output file for test results (JSON)')
    parser.add_argument('--test', '-t', choices=['init', 'status', 'usb', 'safety', 'state', 'sensors', 'ble', 'all'], 
                       default='all', help='Specific test to run')
    
    args = parser.parse_args()
    
    # Handle port argument (either positional or flag)
    port = args.port or args.port_flag
    if not port:
        print("❌ Error: Serial port is required")
        print("Usage: python test_charging_serial.py <port> [options]")
        print("   or: python test_charging_serial.py --port <port> [options]")
        print("\nExamples:")
        print("  python test_charging_serial.py /dev/ttyUSB0")
        print("  python test_charging_serial.py --port /dev/ttyUSB0 --test usb")
        sys.exit(1)
    
    tester = ESP32ChargingTester(port, args.baudrate)
    
    if args.test == 'all':
        results = tester.run_comprehensive_test()
    else:
        if not tester.connect():
            print("❌ Failed to connect to device")
            sys.exit(1)
        
        # Use the port variable instead of args.port
        tester.port = port
        
        try:
            if args.test == 'init':
                results = tester.test_charging_initialization()
            elif args.test == 'status':
                results = tester.test_charging_status_updates()
            elif args.test == 'usb':
                results = tester.test_usb_connection_simulation()
            elif args.test == 'safety':
                results = tester.test_charging_safety_system()
            elif args.test == 'state':
                results = tester.test_charging_state_machine()
            elif args.test == 'sensors':
                results = tester.test_sensor_readings()
            elif args.test == 'ble':
                results = tester.test_ble_integration()
        finally:
            tester.disconnect()
    
    if args.output:
        tester.save_results(args.output)
    
    print("\n🎉 Testing complete!")

if __name__ == "__main__":
    main() 