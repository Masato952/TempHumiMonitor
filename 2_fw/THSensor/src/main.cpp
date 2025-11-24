#include <WiFi.h>
#include <HTTPClient.h>
#include <DHTesp.h>
#include <time.h>

// ----------------------- CONFIG -----------------------
const char* WIFI_SSID = "SGP200W-9FA5-bg";
const char* WIFI_PASSWORD = "UVUZj9VY";

const char* NTP_SERVER = "ntp.nict.jp"; 
const long GMT_OFFSET_SEC = 9 * 3600;  
const int DAYLIGHT_OFFSET_SEC = 0;

const char* SERVER_URL = "https://temp-humi-monitor.vercel.app/api/upload";

DHTesp dht;
const int DHT_PIN = 21; 

// -------------------- FUNCTION DEFINITIONS --------------------
bool connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\nFailed to connect to WiFi");
        return false;
    }
}

bool syncTime() {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
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
    Serial.println("Temperature: " + String(temperature));
    Serial.println("Humidity: " + String(humidity));
    return true;
}

bool uploadData(float temperature, float humidity) {
    if (WiFi.status() != WL_CONNECTED) return false;

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"temp\":" + String(temperature) +
                     ",\"humi\":" + String(humidity) +
                     ",\"ts\":" + String(time(nullptr)) + "}";

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

// 判断是否为 3、6、9… 整点上传时间
bool isUploadHour() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return false;

    if (timeinfo.tm_min == 0 && timeinfo.tm_hour % 3 == 0) {
        Serial.printf("Current time %02d:%02d - Upload allowed\n",
                      timeinfo.tm_hour, timeinfo.tm_min);
        return true;
    }
    return false;
}

// 进入下一个小时的深度睡眠
void sleepUntilNextHour() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    int secondsToNextHour = (60 - timeinfo.tm_min) * 60 - timeinfo.tm_sec;
    Serial.printf("Sleeping for %d seconds until next hour...\n", secondsToNextHour);
    esp_sleep_enable_timer_wakeup(secondsToNextHour * 1000000ULL);
    esp_deep_sleep_start();
}

// -------------------- SETUP --------------------
void setup() {
    Serial.begin(115200);
    dht.setup(DHT_PIN, DHTesp::DHT11);

    if (!connectToWiFi()) {
        Serial.println("Cannot connect to WiFi, entering deep sleep...");
        sleepUntilNextHour();
    }

    if (!syncTime()) {
        Serial.println("Time not synced, proceeding anyway");
    }
}

// -------------------- LOOP --------------------
void loop() {
    if (true){//(isUploadHour()) {
        float temperature, humidity;
        if (readDHT(temperature, humidity)) {
            if (!uploadData(temperature, humidity)) {
                Serial.println("Upload failed");
            }
        }
    }

    // 断开 WiFi 节省功耗
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    sleepUntilNextHour(); // Deep sleep until next hour
}
