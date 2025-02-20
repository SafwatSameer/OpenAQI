#include <Arduino.h>
#include <WiFi.h>
#include "DHT.h"
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>
#include <PMS.h>

#define WIFI_SSID "Your WiFi SSID"
#define WIFI_PASSWORD "Your WiFi Password"

#define PROJECT_ID "wizkit-aqi-project-450806"
#define CLIENT_EMAIL "wizkit-aqi@wizkit-aqi-project-450806.iam.gserviceaccount.com"

const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDu54OaKwpL6y0X\n8+gWUBl6VDAi9mMqqyh8eJ/cfupRf7SYsJKKrENKawQlt+UJiDUw357ZV8CFPZpf\n7/TL6zxPvHag20THZgVF5GNEmf8KEH6UWlCPcIHPaxcuY54Kbd3IqTSu5Qe/sAyR\ndckCsV0JeUsDnWHrWfXy8BB3jqswSqX4rY87Dy97QC2MHqolwPw9//R3UaYfQZLR\n1dlkEHBVJ6Hls36dmgYx2h0ntavBkASl99n2C9EswmnuxjrrcStVPn5E/k8myFyu\nJbs2R0rWOhANadrQ9FBuNOqQUtTZ4vOz4MFtItayvcXlxrziPCxP1VIQEjXJGq84\nX0Gm3EqtAgMBAAECggEAAQcen32aSza/8kvQw/FVUxu9I192wRbb5X5WBq8Pkuqp\nyGJi2a2YVXg/WQjcaEzOJQQcs++Hzr7AAf0t3Va6E9d00qBvoe8356MGbYtdxxlz\nKZS0vyn+/JfK1IxnjHem6vLlH/BDEmpJVpp8sZ2OxqxG9sUo/uyMTKru4FicW3TG\nJ8jKVcBj5xJ+sw/NO3MqeIMjNUomxQIBNy/RVav8ecVgg2QpOfUYp1iSV7ZLWbc9\ngfIRmmVifjUJrBlhbVbHT4ymlGVT6i+oQ9k2z95GJeUoMdZ25oHxwrxHRrq2pFIe\nmotb85olmM+bO0WiWgGlshBRgOnMf6LadQs7FluADQKBgQD6+zn2BWfdNK+fklMN\nX45FQ9l5rPZ2FGQleTVyISHfw7JUJKjn5ql5iHAOYBsK0YsHzVZFuN3UfRNBYI/Q\nl2FkciuagL3DZv/eOlUE4FjlGLi6zM2bMG/ub8tAtrvu5RCugnMPDUihwp4J608Z\nuePwuZ07S7O7qiw7C+JNrAZ5iwKBgQDzrncqSwCSXR5nHr1LOBDjzWkv6DVYfxJn\njTbo6fawWdFPppL0xmJwrPihQ836urw8JdiYh5mHdHzl5B4dUgQIOKw3hABGUjkE\nPaNR9BjA/Mj62c1Xxy5m/0kCegENI+fe0BhJsS0gIvUx3Q8hxqz8gjAuNPC3VkzA\ny3KAIq8jpwKBgBLy7MlaVyEoXSobDhIwaLlqCf1ZxbHckEMUB6j6P2h5ifk4W1hG\n3fzDBFLRhrkmYOzR3/Wbo4PrfYyuEa2aAExpIkmj9q/OEvtgRBL+LTf+ydOwqsVQ\nvuKtphbYtQ+wU7YZOz6jrhwoaW4uSeaTVlZSbUCQ0p0UmApNN2TN3B9JAoGBAJES\n+D0EpBV/PL91zIqAmYWV4B62ZzQFiwZ9SwiFael5v6Rk7i6uASNFx0vzGyk/jRte\nCMJeUGgVUNpo4gjgsCfY3aa7fpJfFzTQIXCEG0yh8mFt09ODcLu/Fk9UJVYWX1AG\nob8VrWDpjwKdsgmQx6IZO+f2eq0bwtwxAz0H6S8XAoGBAJhC88+tZv2NlOSkh5r6\nD8sT9yhuTwVdp/MEw4PIuKww+ZDVjXV9VpBOW4nwyc+txp9YjAc7u4IypBcgVH2O\nVdcYDvjOwNUF/Ac6u9qo7CKRfqrJLm40j4Uh70oMv11i8ggn7+eOHeeXNY07hfo/\n6lFnDKwY/15nZy5zgaU8TKUR\n-----END PRIVATE KEY-----\n";
const char spreadsheetId[] = "1gMAXOSr4SiXFlMr59nLD4_vWFGMjmf5W__bJYHQ9DJE";

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

void tokenStatusCallback(TokenInfo info);

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define PMS_RX 16  // Define RX pin for PMS5003
#define PMS_TX 17  // Define TX pin for PMS5003

PMS pms(Serial2);
PMS::DATA pmsData;

float temp;
float hum;
int pm1_0, pm2_5, pm10;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 21600; // GMT+6 for Bangladesh
const int daylightOffset_sec = 0;

String getFormattedTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Failed to obtain time";
    }
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%B %d, %Y %I:%M %p", &timeinfo);
    return String(timeStringBuff);
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    dht.begin();

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    GSheet.setTokenCallback(tokenStatusCallback);
    GSheet.setPrerefreshSeconds(10 * 60);
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop() {
    bool ready = GSheet.ready();

    if (ready && millis() - lastTime > timerDelay) {
        lastTime = millis();

        FirebaseJson response;
        Serial.println("\nAppend spreadsheet values...");
        Serial.println("----------------------------");

        FirebaseJson valueRange;
        
        temp = dht.readTemperature();
        hum = dht.readHumidity();
        
        pm1_0 = pm2_5 = pm10 = -1;  // Default to -1 if no data is read
        
        if (pms.readUntil(pmsData)) {
            pm1_0 = pmsData.PM_AE_UG_1_0;
            pm2_5 = pmsData.PM_AE_UG_2_5;
            pm10 = pmsData.PM_AE_UG_10_0;
        }

        String formattedTime = getFormattedTime();

        valueRange.add("majorDimension", "COLUMNS");
        valueRange.set("values/[0]/[0]", formattedTime);
        valueRange.set("values/[1]/[0]", temp);
        valueRange.set("values/[2]/[0]", hum);
        valueRange.set("values/[3]/[0]", pm1_0);
        valueRange.set("values/[4]/[0]", pm2_5);
        valueRange.set("values/[5]/[0]", pm10);

        bool success = GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &valueRange);
        if (success) {
            response.toString(Serial, true);
            valueRange.clear();
        } else {
            Serial.println(GSheet.errorReason());
        }
        Serial.println();
        Serial.println(ESP.getFreeHeap());
    }
}

void tokenStatusCallback(TokenInfo info) {
    if (info.status == token_status_error) {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    } else {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}
