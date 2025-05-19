// lib/service/ConfigService.cpp
// ConfigService class implementation
#include "ConfigService.h"
#include <ArduinoJson.h>
#include <FS.h>

String readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return "";
    }
    return file.readString();
}

CertificateCredentialModel ConfigService::getCertificateCredential()
{
    String ca = readFile(fileSystem, "/certs/aws_cert_ca.pem");
    String certificate = readFile(fileSystem, "/certs/certificate.pem.crt");
    String privateKey = readFile(fileSystem, "/certs/private.pem.key");
    return CertificateCredentialModel(ca, certificate, privateKey);
}

WifiCredentialModel ConfigService::getWifiCredential()
{
    File wifiConfigFile = fileSystem.open("/wifi_config.json", "r");
    if (!wifiConfigFile)
    {
        Serial.println("Failed to open wifi_config.json file");
        return WifiCredentialModel();
    }
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, wifiConfigFile);
    if (error)
    {
        Serial.println("Failed to read file, using default configuration");
        wifiConfigFile.close();
        return WifiCredentialModel();
    }
    String ssid = doc["ssid"];
    String password = doc["password"];
    wifiConfigFile.close();
    return WifiCredentialModel(ssid, password);
}

MqttCredentialModel ConfigService::getMqttCredential()
{
    File mqttConfigFile = fileSystem.open("/mqtt_config.json", "r");
    if (!mqttConfigFile)
    {
        Serial.println("Failed to open mqtt_config.json file");
        return MqttCredentialModel();
    }
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, mqttConfigFile);
    if (error)
    {
        Serial.println("Failed to read file, using default configuration");
        mqttConfigFile.close();
        return MqttCredentialModel();
    }
    String host = doc["host"];
    int port = doc["port"];
    String clientId = doc["clientId"];
    String publishTopic = doc["publishTopic"];
    mqttConfigFile.close();
    return MqttCredentialModel(port, host, clientId, publishTopic);
}
