#pragma once
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

//hardware settings
#define TF_PW_CTL   42
#define SD_MOSI     40
#define SD_MISO     13
#define SD_SCK      39
#define SD_CS       10

struct ConfigStruct {
    //load from sd card
    String wifi_ssid;
    String wifi_password;
    //ntp
    String ntp_server;
    long gmt_offset;
    long daylight_offset;
    //upload
    String upload_server;
    String openweather_apikey;
    String loc_city;
    String loc_lat;
    String loc_lon;
    String ipinfo_apikey;
    //status
    bool sd_failed;
    bool nt_status;
};

// 全局配置对象
extern ConfigStruct cfgs;

// SD 卡相关函数声明
bool loadConfig(const char* filename);
int sd_init();
void sdcard_init();
