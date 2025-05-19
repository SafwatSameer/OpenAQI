#ifndef PAYLOADMODEL_H
#define PAYLOADMODEL_H

#include <Arduino.h>
#include <ArduinoJson.h>

class PayloadModel
{
private:
    String clientId;
    bool isClientIdValid;
    float humidity;
    bool isHumidityValid;
    float temperature;
    bool isTemperatureValid;
    // Add new fields for PMS data
    uint16_t pm1;
    bool isPm1Valid;
    uint16_t pm25;
    bool isPm25Valid;
    uint16_t pm10;
    bool isPm10Valid;

    // Date and time fields
    String date;
    bool isDateValid;
    String time;
    bool isTimeValid;

public:
    PayloadModel()
    {
        clientId = "";
        isClientIdValid = false;
        humidity = 0;
        isHumidityValid = false;
        temperature = 0;
        isTemperatureValid = false;
        pm1 = 0;
        isPm1Valid = false;
        pm25 = 0;
        isPm25Valid = false;
        pm10 = 0;
        isPm10Valid = false;
        date = "";
        isDateValid = false;
        time = "";
        isTimeValid = false;
    };
    void setClientId(String clientId, bool isClientIdValid)
    {
        this->clientId = clientId;
        this->isClientIdValid = isClientIdValid;
    };
    void setHumidity(float humidity, bool isHumidityValid)
    {
        this->humidity = humidity;
        this->isHumidityValid = isHumidityValid;
    };
    void setTemperature(float temperature, bool isTemperatureValid)
    {
        this->temperature = temperature;
        this->isTemperatureValid = isTemperatureValid;
    };
    // Add setters for PM data
    void setPM1(uint16_t pm1, bool isPm1Valid)
    {
        this->pm1 = pm1;
        this->isPm1Valid = isPm1Valid;
    };
    void setPM25(uint16_t pm25, bool isPm25Valid)
    {
        this->pm25 = pm25;
        this->isPm25Valid = isPm25Valid;
    };
    void setPM10(uint16_t pm10, bool isPm10Valid)
    {
        this->pm10 = pm10;
        this->isPm10Valid = isPm10Valid;
    };

    // Date and time setters
    void setDate(String date, bool isDateValid)
    {
        this->date = date;
        this->isDateValid = isDateValid;
    };
    void setTime(String time, bool isTimeValid)
    {
        this->time = time;
        this->isTimeValid = isTimeValid;
    };

    char *toJson()
    {
        static char buffer[512];  // Increased buffer size to accommodate additional data
        DynamicJsonDocument doc(384);  // Increased size for additional JSON fields
        if (this->isClientIdValid)
        {
            doc["clientId"] = this->clientId;
        }
        else
        {
            doc["clientId"] = nullptr;
        }
        if (this->isHumidityValid)
        {
            doc["humidity"] = this->humidity;
        }
        else
        {
            doc["humidity"] = nullptr;
        }
        if (this->isTemperatureValid)
        {
            doc["temperature"] = this->temperature;
        }
        else
        {
            doc["temperature"] = nullptr;
        }
        // Add PM data to JSON
        if (this->isPm1Valid)
        {
            doc["pm1"] = this->pm1;
        }
        else
        {
            doc["pm1"] = nullptr;
        }
        if (this->isPm25Valid)
        {
            doc["pm25"] = this->pm25;
        }
        else
        {
            doc["pm25"] = nullptr;
        }
        if (this->isPm10Valid)
        {
            doc["pm10"] = this->pm10;
        }
        else
        {
            doc["pm10"] = nullptr;
        }

        // Date and time to JSON
        if (this->isDateValid)
        {
            doc["date"] = this->date;
        }
        else
        {
            doc["date"] = nullptr;
        }
        if (this->isTimeValid)
        {
            doc["time"] = this->time;
        }
        else
        {
            doc["time"] = nullptr;
        }

        serializeJson(doc, buffer);
        return buffer;
    };
};
#endif
