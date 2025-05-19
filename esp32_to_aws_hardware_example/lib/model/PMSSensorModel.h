// Updated PMSSensorModel.h

#ifndef PMSSENSORMODEL_H
#define PMSSENSORMODEL_H

#include <Arduino.h>

// PMS5003 data structure
struct pms5003data {
  uint16_t framelen;       // Frame length
  uint16_t pm10_standard;  // PM1.0 concentration in μg/m3 (standard)
  uint16_t pm25_standard;  // PM2.5 concentration in μg/m3 (standard)
  uint16_t pm100_standard; // PM10 concentration in μg/m3 (standard)
  uint16_t pm10_env;       // PM1.0 concentration in μg/m3 (environmental)
  uint16_t pm25_env;       // PM2.5 concentration in μg/m3 (environmental)
  uint16_t pm100_env;      // PM10 concentration in μg/m3 (environmental)
  uint16_t particles_03um; // Particles >0.3μm / 0.1L air
  uint16_t particles_05um; // Particles >0.5μm / 0.1L air
  uint16_t particles_10um; // Particles >1.0μm / 0.1L air
  uint16_t particles_25um; // Particles >2.5μm / 0.1L air
  uint16_t particles_50um; // Particles >5.0μm / 0.1L air
  uint16_t particles_100um;// Particles >10μm / 0.1L air
  uint16_t unused;         // Unused
  uint16_t checksum;       // Checksum
};

class PMSSensor {
private:
  Stream* serial;
  pms5003data data;
  bool dataReady;
  unsigned long lastReadAttempt;
  const unsigned long READ_TIMEOUT = 1000; // 1 second timeout between read attempts

public:
  PMSSensor(Stream* serial) : serial(serial), dataReady(false), lastReadAttempt(0) {
    // Clear any existing data in buffer
    memset(&data, 0, sizeof(data));
  }

  // Initialize the sensor and clear the buffer
  void begin() {
    // Clear the serial buffer
    while (serial->available()) {
      serial->read();
    }
    Serial.println("PMS5003 sensor initialized");
  }

  bool readData() {
    unsigned long currentMillis = millis();
    
    // Don't try to read too frequently to avoid buffer issues
    if (currentMillis - lastReadAttempt < READ_TIMEOUT) {
      return false;
    }
    
    lastReadAttempt = currentMillis;
    
    // Make sure there's enough data available
    if (serial->available() < 32) {
      return false;
    }
    
    // Look for the start byte sequence (0x42, 0x4D)
    uint8_t startByte;
    bool foundStart = false;
    unsigned long startTime = millis();
    
    // Search for the start bytes with a timeout
    while (millis() - startTime < 500) {
      if (!serial->available()) {
        delay(10);
        continue;
      }
      
      startByte = serial->read();
      if (startByte == 0x42) {
        if (serial->available() && serial->peek() == 0x4D) {
          serial->read(); // consume the 0x4D byte
          foundStart = true;
          break;
        }
      }
    }
    
    if (!foundStart) {
      Serial.println("PMS5003: Failed to find start bytes");
      return false;
    }
    
    // Wait until we have at least 30 more bytes (32 - 2 start bytes)
    startTime = millis();
    while (serial->available() < 30) {
      if (millis() - startTime > 1000) {  // 1 second timeout
        Serial.println("PMS5003: Timeout waiting for data");
        return false;
      }
      delay(10);
    }
    
    // Read the remaining 30 bytes
    uint8_t buffer[30];
    serial->readBytes(buffer, 30);
    
    // Calculate checksum
    uint16_t checksum = 0x42 + 0x4D;  // Include the start bytes
    for (uint8_t i = 0; i < 28; i++) {
      checksum += buffer[i];
    }
    
    uint16_t receivedChecksum = (buffer[28] << 8) | buffer[29];
    if (checksum != receivedChecksum) {
      Serial.print("PMS5003 checksum failure. Expected: ");
      Serial.print(checksum);
      Serial.print(", Received: ");
      Serial.println(receivedChecksum);
      return false;
    }
    
    // Parse the data
    data.framelen = (buffer[0] << 8) | buffer[1];
    data.pm10_standard = (buffer[2] << 8) | buffer[3];
    data.pm25_standard = (buffer[4] << 8) | buffer[5];
    data.pm100_standard = (buffer[6] << 8) | buffer[7];
    data.pm10_env = (buffer[8] << 8) | buffer[9];
    data.pm25_env = (buffer[10] << 8) | buffer[11];
    data.pm100_env = (buffer[12] << 8) | buffer[13];
    data.particles_03um = (buffer[14] << 8) | buffer[15];
    data.particles_05um = (buffer[16] << 8) | buffer[17];
    data.particles_10um = (buffer[18] << 8) | buffer[19];
    data.particles_25um = (buffer[20] << 8) | buffer[21];
    data.particles_50um = (buffer[22] << 8) | buffer[23];
    data.particles_100um = (buffer[24] << 8) | buffer[25];
    data.unused = (buffer[26] << 8) | buffer[27];
    data.checksum = receivedChecksum;
    
    // Print debug values
    
    
    dataReady = true;
    return true;
  }

  uint16_t getPM1() {
    return dataReady ? data.pm10_standard : 0;
  }

  uint16_t getPM25() {
    return dataReady ? data.pm25_standard : 0;
  }

  uint16_t getPM10() {
    return dataReady ? data.pm100_standard : 0;
  }

  bool isDataReady() {
    return dataReady;
  }

  void resetDataReady() {
    dataReady = false;
  }
  
  // Flush any pending data in the serial buffer
  void flushInput() {
    while (serial->available()) {
      serial->read();
    }
  }
};

#endif // PMSSENSORMODEL_H