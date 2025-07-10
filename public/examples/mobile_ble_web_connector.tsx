import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  TouchableOpacity,
  StyleSheet,
  Alert,
  Linking,
  ActivityIndicator,
} from 'react-native';
import { BleManager, Device, Characteristic } from 'react-native-ble-plx';

// BLE Service and Characteristic UUIDs
const SERVICE_UUID = '19B10000-E8F2-537E-4F6C-D104768A1214';
const WEB_URL_UUID = '19B10022-E8F2-537E-4F6C-D104768A1214';
const WEB_CONTENT_UUID = '19B10023-E8F2-537E-4F6C-D104768A1214';
const DEVICE_STATUS_UUID = '19B10007-E8F2-537E-4F6C-D104768A1214';

// Device identification
const DEVICE_NAME = 'OpenGlass Camera';

interface DeviceStatus {
  isScanning: boolean;
  isConnected: boolean;
  deviceFound: boolean;
  webUrl: string | null;
  deviceStatusValue: number | null;
}

const BLEWebConnector: React.FC = () => {
  const [bleManager] = useState(new BleManager());
  const [device, setDevice] = useState<Device | null>(null);
  const [status, setStatus] = useState<DeviceStatus>({
    isScanning: false,
    isConnected: false,
    deviceFound: false,
    webUrl: null,
    deviceStatusValue: null,
  });

  useEffect(() => {
    // Initialize BLE manager
    const subscription = bleManager.onStateChange((state) => {
      if (state === 'PoweredOn') {
        console.log('BLE is powered on');
      }
    }, true);

    return () => {
      subscription.remove();
      bleManager.destroy();
    };
  }, [bleManager]);

  const scanForDevice = async () => {
    setStatus(prev => ({ ...prev, isScanning: true, deviceFound: false }));
    
    try {
      // Start scanning
      bleManager.startDeviceScan(null, null, (error, device) => {
        if (error) {
          console.error('Scan error:', error);
          setStatus(prev => ({ ...prev, isScanning: false }));
          Alert.alert('Scan Error', error.message);
          return;
        }

        if (device && device.name && device.name.includes(DEVICE_NAME)) {
          console.log('Found device:', device.name, device.id);
          setDevice(device);
          setStatus(prev => ({ 
            ...prev, 
            isScanning: false, 
            deviceFound: true 
          }));
          bleManager.stopDeviceScan();
        }
      });

      // Stop scanning after 10 seconds
      setTimeout(() => {
        bleManager.stopDeviceScan();
        setStatus(prev => {
          if (prev.isScanning && !prev.deviceFound) {
            Alert.alert('Device Not Found', `${DEVICE_NAME} not found. Make sure it's powered on and advertising.`);
          }
          return { ...prev, isScanning: false };
        });
      }, 10000);
    } catch (error) {
      console.error('Scan failed:', error);
      setStatus(prev => ({ ...prev, isScanning: false }));
      Alert.alert('Scan Failed', 'Failed to start scanning for devices.');
    }
  };

  const connectToDevice = async () => {
    if (!device) return;

    try {
      console.log('Connecting to device...');
      const connectedDevice = await device.connect();
      
      console.log('Discovering services...');
      await connectedDevice.discoverAllServicesAndCharacteristics();
      
      setDevice(connectedDevice);
      setStatus(prev => ({ ...prev, isConnected: true }));
      
      // Read web URL immediately after connecting
      await readWebUrl();
      await readDeviceStatus();
      
      Alert.alert('Connected', 'Successfully connected to the camera!');
    } catch (error) {
      console.error('Connection failed:', error);
      Alert.alert('Connection Failed', 'Failed to connect to the device.');
    }
  };

  const disconnectFromDevice = async () => {
    if (!device) return;

    try {
      await device.cancelConnection();
      setDevice(null);
      setStatus(prev => ({ 
        ...prev, 
        isConnected: false, 
        deviceFound: false,
        webUrl: null,
        deviceStatusValue: null
      }));
      Alert.alert('Disconnected', 'Disconnected from the camera.');
    } catch (error) {
      console.error('Disconnection failed:', error);
      Alert.alert('Disconnection Failed', 'Failed to disconnect from the device.');
    }
  };

  const readWebUrl = async () => {
    if (!device) return;

    try {
      const characteristic = await device.readCharacteristicForService(
        SERVICE_UUID,
        WEB_URL_UUID
      );
      
      if (characteristic.value) {
        const webUrl = Buffer.from(characteristic.value, 'base64').toString('utf-8');
        console.log('Web URL:', webUrl);
        setStatus(prev => ({ ...prev, webUrl }));
      }
    } catch (error) {
      console.error('Failed to read web URL:', error);
      Alert.alert('Read Error', 'Failed to read web URL from device.');
    }
  };

  const readDeviceStatus = async () => {
    if (!device) return;

    try {
      const characteristic = await device.readCharacteristicForService(
        SERVICE_UUID,
        DEVICE_STATUS_UUID
      );
      
      if (characteristic.value) {
        const statusBuffer = Buffer.from(characteristic.value, 'base64');
        const deviceStatusValue = statusBuffer.readUInt8(0);
        console.log('Device Status:', deviceStatusValue);
        setStatus(prev => ({ ...prev, deviceStatusValue }));
      }
    } catch (error) {
      console.error('Failed to read device status:', error);
      Alert.alert('Read Error', 'Failed to read device status.');
    }
  };

  const openWebInterface = async () => {
    if (!status.webUrl) {
      Alert.alert('No URL', 'Web interface URL not available. Please read the URL first.');
      return;
    }

    try {
      const supported = await Linking.canOpenURL(status.webUrl);
      if (supported) {
        await Linking.openURL(status.webUrl);
      } else {
        Alert.alert('Cannot Open URL', 'Unable to open the web interface URL.');
      }
    } catch (error) {
      console.error('Failed to open URL:', error);
      Alert.alert('Open Error', 'Failed to open the web interface.');
    }
  };

  const getDeviceStatusText = (statusValue: number | null) => {
    if (statusValue === null) return 'Unknown';
    
    const statusMap: { [key: number]: string } = {
      0x01: 'Initializing',
      0x02: 'Ready',
      0x03: 'Camera Active',
      0x04: 'Recording',
      0x05: 'Photo Capture',
      0x06: 'Streaming',
      0x07: 'Processing',
      0x08: 'Error',
      0x09: 'Low Battery',
      0x0A: 'Charging',
      0x0B: 'WiFi Init',
      0x0C: 'WiFi Connecting',
      0x0D: 'WiFi Connected',
      0x0E: 'WiFi AP Mode',
      0x0F: 'WiFi Error',
    };
    
    return statusMap[statusValue] || `Unknown (0x${statusValue.toString(16).toUpperCase()})`;
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>ESP32S3 Camera BLE Connector</Text>
      
      <View style={styles.statusContainer}>
        <Text style={styles.statusText}>
          Status: {status.isConnected ? 'Connected' : status.deviceFound ? 'Found' : 'Disconnected'}
        </Text>
        {status.deviceStatusValue !== null && (
          <Text style={styles.statusText}>
            Device: {getDeviceStatusText(status.deviceStatusValue)}
          </Text>
        )}
        {status.webUrl && (
          <Text style={styles.urlText}>URL: {status.webUrl}</Text>
        )}
      </View>

      <View style={styles.buttonContainer}>
        {!status.deviceFound && (
          <TouchableOpacity
            style={[styles.button, styles.scanButton]}
            onPress={scanForDevice}
            disabled={status.isScanning}
          >
            {status.isScanning ? (
              <ActivityIndicator color="#ffffff" />
            ) : (
              <Text style={styles.buttonText}>Scan for Camera</Text>
            )}
          </TouchableOpacity>
        )}

        {status.deviceFound && !status.isConnected && (
          <TouchableOpacity
            style={[styles.button, styles.connectButton]}
            onPress={connectToDevice}
          >
            <Text style={styles.buttonText}>Connect</Text>
          </TouchableOpacity>
        )}

        {status.isConnected && (
          <>
            <TouchableOpacity
              style={[styles.button, styles.refreshButton]}
              onPress={readWebUrl}
            >
              <Text style={styles.buttonText}>Refresh URL</Text>
            </TouchableOpacity>

            <TouchableOpacity
              style={[styles.button, styles.refreshButton]}
              onPress={readDeviceStatus}
            >
              <Text style={styles.buttonText}>Refresh Status</Text>
            </TouchableOpacity>

            {status.webUrl && (
              <TouchableOpacity
                style={[styles.button, styles.openButton]}
                onPress={openWebInterface}
              >
                <Text style={styles.buttonText}>Open Web Interface</Text>
              </TouchableOpacity>
            )}

            <TouchableOpacity
              style={[styles.button, styles.disconnectButton]}
              onPress={disconnectFromDevice}
            >
              <Text style={styles.buttonText}>Disconnect</Text>
            </TouchableOpacity>
          </>
        )}
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 20,
    backgroundColor: '#f5f5f5',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 30,
    textAlign: 'center',
    color: '#333',
  },
  statusContainer: {
    backgroundColor: '#fff',
    padding: 20,
    borderRadius: 10,
    marginBottom: 30,
    minWidth: '100%',
    alignItems: 'center',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  statusText: {
    fontSize: 16,
    marginBottom: 5,
    color: '#333',
  },
  urlText: {
    fontSize: 14,
    color: '#666',
    marginTop: 10,
    textAlign: 'center',
  },
  buttonContainer: {
    width: '100%',
    alignItems: 'center',
  },
  button: {
    paddingHorizontal: 30,
    paddingVertical: 15,
    borderRadius: 8,
    marginVertical: 8,
    minWidth: '80%',
    alignItems: 'center',
  },
  scanButton: {
    backgroundColor: '#007AFF',
  },
  connectButton: {
    backgroundColor: '#34C759',
  },
  refreshButton: {
    backgroundColor: '#FF9500',
  },
  openButton: {
    backgroundColor: '#5856D6',
  },
  disconnectButton: {
    backgroundColor: '#FF3B30',
  },
  buttonText: {
    color: '#ffffff',
    fontSize: 16,
    fontWeight: '600',
  },
});

export default BLEWebConnector; 