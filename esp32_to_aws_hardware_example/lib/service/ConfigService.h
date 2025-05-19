// lib/service/ConfigService.h
// ConfigService class definition.

#ifndef CONFIGSERVICE_H
#define CONFIGSERVICE_H

#include <Arduino.h>
#include "../model/CertificateCredentialModel.h"
#include "../model/MqttCredentialModel.h"
#include "../model/WifiCredentialModel.h"
#include <FS.h>

class ConfigService
{
private:
    fs::FS &fileSystem;

public:
    ConfigService(fs::FS &fileSystem) : fileSystem(fileSystem){};

    WifiCredentialModel getWifiCredential();
    MqttCredentialModel getMqttCredential();
    CertificateCredentialModel getCertificateCredential();
};

#endif

