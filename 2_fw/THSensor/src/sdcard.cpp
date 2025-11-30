#include "sdcard.h"

SPIClass SD_SPI; // 自定义 SPI，如果用默认 SPI 可以删掉

ConfigStruct cfgs;

bool loadConfig(const char* filename) {
    File file = SD.open(filename);
    if (!file) {
        Serial.println("Failed to open config file");
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }
    // Use value from JSON, or default if missing
    cfgs.wifi_ssid = doc["wifi"]["ssid"] | "";
    cfgs.wifi_password = doc["wifi"]["password"] | "";
    cfgs.ntp_server      = doc["ntp"]["server"].as<String>();
    cfgs.gmt_offset = (doc["ntp"]["gmt_offset"] | 9) * 3600;
    cfgs.daylight_offset = doc["ntp"]["daylight_offset"] | 0;
    cfgs.upload_server = doc["upload_server"]["url"].as<String>();
    cfgs.openweather_apikey = doc["openweather"]["apikey"] | "";
    cfgs.loc_city = doc["location"]["city"] | "";
    cfgs.loc_lat = doc["location"]["lat"] | "";
    cfgs.loc_lon = doc["location"]["lon"] | "";
    cfgs.ipinfo_apikey = doc["location"]["ipinfo_apikey"] | "";

    file.close();
    return true;
}

int sd_init() {
    SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SD_SPI, 80000000)) {  // ESP32-S3 默认 SPI
        Serial.println("Card Mount Failed");
        return 1;
    }

    Serial.println("Card Mount Successed");
    loadConfig("/config.json");
    Serial.println("TF Card init over.");
    return 0;
}

void sdcard_init() {
    pinMode(TF_PW_CTL, OUTPUT);
    digitalWrite(TF_PW_CTL, HIGH);

    if (sd_init() == 0) {
        cfgs.sd_failed = false;
        Serial.println("TF init success");
    } else {
        cfgs.sd_failed = true;
        Serial.println("TF init fail");
    }
}
