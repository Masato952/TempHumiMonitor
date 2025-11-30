#include <WiFi.h>
#include <HTTPClient.h>
#include <DHTesp.h>
#include <time.h>

#include "sdcard.h"

// ----------------------- CONFIG -----------------------

DHTesp dht;
const int DHT_PIN = 21; 

// -------------------- FUNCTION DEFINITIONS --------------------
bool connectWiFi(const char* wifi_ssid, const char* wifi_password) {
    bool status;
    unsigned long startAttemptTime = millis();
    const unsigned long WIFI_TIMEOUT_MS = 180000; // 3 minutes timeout

    WiFi.begin(wifi_ssid, wifi_password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS)
    {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to WiFi. ");
        status = false;
    }else{
        Serial.println("\nWiFi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        status = true;
    }
    return status;
}

bool syncTime(long gmtOffsetSec, long daylightOffsetSec, const char* ntpServer) {
    configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
    struct tm timeinfo;
    int retry = 0;

    while (!getLocalTime(&timeinfo) && retry < 20) {
        Serial.println("Waiting for NTP...");
        retry++;
        delay(500);
    }

    if (retry < 20) {
        Serial.printf("[%04d-%02d-%02d %02d:%02d:%02d] Time synced OK\n",
                      timeinfo.tm_year + 1900,
                      timeinfo.tm_mon + 1,
                      timeinfo.tm_mday,
                      timeinfo.tm_hour,
                      timeinfo.tm_min,
                      timeinfo.tm_sec);
        return true;
    } else {
        Serial.println("Failed to obtain time");
        return false;
    }
}

bool readDHT(float &temperature, float &humidity) {
    TempAndHumidity data = dht.getTempAndHumidity();
    if (isnan(data.temperature) || isnan(data.humidity)) {
        Serial.println("DHT read error");
        return false;
    }
    temperature = data.temperature;
    humidity = data.humidity;
    Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
    return true;
}

bool uploadData(const char* serverUrl, float temperature, float humidity) {
    if (WiFi.status() != WL_CONNECTED) return false;

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"temp\":" + String(temperature) +
                     ",\"humi\":" + String(humidity) + "}";

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
        Serial.println("Server response: " + http.getString());
        http.end();
        return true;
    } else {
        Serial.print("HTTP POST failed: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}

// Deep sleep 到下一个指定间隔（3 小时）
void sleepUntilNextUpload() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    int secondsToNext = 3600; // 1 小时
    Serial.printf("Sleeping for %d seconds until next upload...\n", secondsToNext);
    esp_sleep_enable_timer_wakeup((uint64_t)secondsToNext * 1000000ULL);
    esp_deep_sleep_start();
}

// -------------------- SETUP --------------------
void setup() {
    Serial.begin(115200);
    dht.setup(DHT_PIN, DHTesp::DHT11);
    sdcard_init();
    if (!connectWiFi(cfgs.wifi_ssid.c_str(),cfgs.wifi_password.c_str())) {
        Serial.println("Cannot connect to WiFi, entering deep sleep...");
        sleepUntilNextUpload();
    }

    if (!syncTime(cfgs.gmt_offset,cfgs.daylight_offset,cfgs.ntp_server.c_str())) {
        Serial.println("Time not synced, proceeding anyway");
    }

    // 启动立即上传一次
    float temperature, humidity;
    if (readDHT(temperature, humidity)) {
        if (!uploadData(cfgs.upload_server.c_str(),temperature, humidity)) {
            Serial.println("Upload failed");
        }
    }

    // 上传完成，断开 WiFi 节省功耗
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    // Deep Sleep 1 小时
    sleepUntilNextUpload();
}

// -------------------- LOOP --------------------
void loop() {
    // loop 不需要做任何事，MCU 已经 Deep Sleep
}
