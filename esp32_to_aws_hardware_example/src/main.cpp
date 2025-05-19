#define LED_PIN 2
#define DHT_PIN 15
#define DHT_TYPE DHT11
// Define PMS5003 serial pins
#define PMS_RX 16 // Connect PMS5003 TX to ESP32 GPIO16
#define PMS_TX 17 // Connect PMS5003 RX to ESP32 GPIO17

unsigned long previousSensorMillis = 0;
const long sensorInterval = 1800000;

#include <Arduino.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "../service/ConfigService.h"
#include "../model/PayloadModel.h"
#include "../model/PMSSensorModel.h"  // Include the PMS sensor model
#include <DHT.h>
#include <HardwareSerial.h>  // For using hardware serial with PMS5003
#include <time.h>  // For time synchronization via NTP

WiFiClientSecure espClient;
PubSubClient client(espClient);
MqttCredentialModel mqttCredential;
WifiCredentialModel wifiCredential;
CertificateCredentialModel certificateCredential;
PayloadModel payloadModel;
char *payload;
DHT dht(DHT_PIN, DHT_TYPE);

// Create a hardware serial for PMS5003 (UART2 on ESP32)
HardwareSerial pmsSensorSerial(2);  // Use UART2 (ESP32 has 3 hardware serial ports)
PMSSensor pmsSensor(&pmsSensorSerial);

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 21600;     // GMT+6 for Dhaka (6 hours = 6*60*60 = 21600 seconds)
const int   daylightOffset_sec = 0;    // No daylight saving in Bangladesh

// Function prototypes
void setupWiFi();
void reconnectMQTT();
void blinkLED(int times);
void setupTime();
bool getFormattedTime(char* dateStr, size_t dateSize, char* timeStr, size_t timeSize);

void setup()
{
    Serial.begin(115200);
    delay(1000); // Give the serial port time to initialize
    Serial.println("Starting...");
    
    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    blinkLED(2); // Indicate startup
    
    // Initialize filesystem
    if (!LittleFS.begin(true)) // true for format_if_failed
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        blinkLED(5); // Error indicator
        delay(1000);
        ESP.restart();
        return;
    }

    // Load configuration
    ConfigService configService(LittleFS);
    
    // Read certificates first
    certificateCredential = configService.getCertificateCredential();
    if (certificateCredential.isEmpty())
    {
        Serial.println("Certificate credential is empty");
        blinkLED(5);
        return;
    }
    
    wifiCredential = configService.getWifiCredential();
    if (wifiCredential.isEmpty())
    {
        Serial.println("Wifi credential is empty");
        blinkLED(3);
        return;
    }

    mqttCredential = configService.getMqttCredential();
    if (mqttCredential.isEmpty())
    {
        Serial.println("Mqtt credential is empty");
        blinkLED(4);
        return;
    }

    // Initialize the PMS5003 serial (9600 is the standard baud rate for PMS5003)
    pmsSensorSerial.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
    delay(1000); // Give PMS sensor time to initialize
    pmsSensor.begin(); // Initialize the sensor and flush any initial data
    
    // Initialize DHT sensor
    dht.begin();
    
    // Connect to WiFi
    setupWiFi();

    // Configure and synchronize time with NTP server
    setupTime();
    
    // Set up MQTT client
    client.setServer(mqttCredential.host.c_str(), mqttCredential.port);
    reconnectMQTT();
    
    // Initialize payload model
    payloadModel = PayloadModel();
    payloadModel.setClientId(mqttCredential.clientId, true);
    
    Serial.println("Setup complete");
    blinkLED(1); // Indicate successful setup
}

void loop()
{
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost. Reconnecting...");
        setupWiFi();
    }
    
    // Check MQTT connection
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();
    
    unsigned long currentMillis = millis();
    
    // Try to read PMS5003 data continuously
    if (pmsSensor.readData()) {
        // Data read successfully - no need to do anything here, 
        // data will be sent in the regular interval
    }
    
    // Send data at regular intervals
    if (currentMillis - previousSensorMillis >= sensorInterval)
    {
        previousSensorMillis = currentMillis;
        digitalWrite(LED_PIN, HIGH); // LED on during data transmission
        
        // Read DHT sensor
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
        
        // Get PM data from sensor
        uint16_t pm1 = pmsSensor.getPM1();
        uint16_t pm25 = pmsSensor.getPM25();
        uint16_t pm10 = pmsSensor.getPM10();

        // Get current time
        char dateStr[11]; // YYYY-MM-DD + null terminator
        char timeStr[9];  // HH:MM:SS + null terminator
        
        bool timeSuccess = getFormattedTime(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));
        
        // Debug output for sensor data and time
        
        
        // Set the data in payload model
        payloadModel.setHumidity(humidity, !isnan(humidity));
        payloadModel.setTemperature(temperature, !isnan(temperature));
        payloadModel.setPM1(pm1, pmsSensor.isDataReady());
        payloadModel.setPM25(pm25, pmsSensor.isDataReady());
        payloadModel.setPM10(pm10, pmsSensor.isDataReady());

        // Set date and time - convert char arrays to Strings
        payloadModel.setDate(String(dateStr), timeSuccess);
        payloadModel.setTime(String(timeStr), timeSuccess);
        
        // Reset PMS data ready flag
        pmsSensor.resetDataReady();
        
        // Serialize and publish
        payload = payloadModel.toJson();
        Serial.println("Publishing MQTT message:");
        Serial.println(payload);
        
        // Make sure we're publishing to the correct topic (esp32/sensor/test)
        if (client.publish(mqttCredential.publishTopic.c_str(), payload)) {
            Serial.println("Publish successful");
            blinkLED(1); // Indicate successful publish
        } else {
            Serial.println("Publish failed");
            blinkLED(2); // Indicate failed publish
        }
        
        digitalWrite(LED_PIN, LOW); // Turn off LED
    }
}

// Helper function to get formatted time
bool getFormattedTime(char* dateStr, size_t dateSize, char* timeStr, size_t timeSize) {
    struct tm timeinfo;
    
    if(!getLocalTime(&timeinfo)) {
        strcpy(dateStr, "0000-00-00");
        strcpy(timeStr, "00:00:00");
        Serial.println("Failed to obtain time");
        return false;
    }
    
    strftime(dateStr, dateSize, "%Y-%m-%d", &timeinfo);
    strftime(timeStr, timeSize, "%H:%M:%S", &timeinfo);
    return true;
}

// Setup WiFi connection
void setupWiFi() {
    Serial.print("Connecting to WiFi SSID: ");
    Serial.println(wifiCredential.ssid.c_str());
    
    WiFi.begin(wifiCredential.ssid.c_str(), wifiCredential.password.c_str());
    
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
        Serial.print(".");
        attempt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi after multiple attempts");
        blinkLED(3);
        delay(1000);
        ESP.restart(); // Restart the ESP32
    }
}

// Reconnect to MQTT broker
void reconnectMQTT() {
    Serial.println("Setting up certificates...");
    
    // Set the certificates to the client
    espClient.setCACert(certificateCredential.ca.c_str());
    espClient.setCertificate(certificateCredential.certificate.c_str());
    espClient.setPrivateKey(certificateCredential.privateKey.c_str());
    
    Serial.println("Connecting to AWS IoT...");
    
    int attempts = 0;
    while (!client.connected() && attempts < 5) {
        attempts++;
        Serial.print("MQTT connection attempt ");
        Serial.print(attempts);
        Serial.print("...");
        
        if (client.connect(mqttCredential.clientId.c_str())) {
            Serial.println("Connected to AWS IoT");
            blinkLED(2); // Indicate successful connection
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying...");
            blinkLED(4); // Indicate connection failure
            delay(2000);
        }
    }
    
    if (!client.connected()) {
        Serial.println("Failed to connect to MQTT after multiple attempts");
        Serial.println("Restarting ESP32...");
        delay(1000);
        ESP.restart();
    }
}

// Utility function to blink LED
void blinkLED(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
}

// Setup time synchronization via NTP
void setupTime() {
    Serial.println("Setting up time synchronization...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Wait for time to be synchronized
    struct tm timeinfo;
    int retries = 0;
    while(!getLocalTime(&timeinfo) && retries < 10) {
        Serial.println("Waiting for time synchronization...");
        delay(1000);
        retries++;
    }
    
    if(retries < 10) {
        Serial.println("Time synchronized successfully");
        char timeString[50];
        strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
        Serial.print("Current time: ");
        Serial.println(timeString);
    } else {
        Serial.println("Failed to synchronize time after multiple attempts");
        // Continue anyway - we'll use a placeholder time
    }
}