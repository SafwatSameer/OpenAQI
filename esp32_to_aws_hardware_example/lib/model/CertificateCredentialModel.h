// lib/model/CertificateCredentialModel.h
// CertificateCredentialModel definition.

#ifndef CERTIFICATECREDENTIALMODEL_H
#define CERTIFICATECREDENTIALMODEL_H

#include <Arduino.h>

class CertificateCredentialModel
{
public:
    String ca;
    String certificate;
    String privateKey;

    CertificateCredentialModel() : ca(""), certificate(""), privateKey(""){};
    CertificateCredentialModel(String ca, String certificate, String privateKey) : ca(ca), certificate(certificate), privateKey(privateKey){};

    bool isEmpty()
    {
        return ca == "" || certificate == "" || privateKey == "";
    }
};

#endif
