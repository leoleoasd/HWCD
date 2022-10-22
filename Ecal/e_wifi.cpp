#include "e_wifi.h"
#include <Arduino.h>
#include <nvs_flash.h>

#include <WiFi.h>
#include "init.h"

static const char *TAG = "ECal Wifi";

void wifi_init() {
  size_t length;
  String ssid = nvs_read_string("ssid");
  String pass = nvs_read_string("pass");
  bool has_ssid = !ssid.isEmpty() && !pass.isEmpty();
  IPAddress  ip;
  if (has_ssid) {
    // ssid found, connect to WiFi
    ESP_LOGI(TAG, "ssid: %s, pass: %s", ssid.c_str(), pass.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      ESP_LOGE(TAG, "Wifi Failed");
      ESP_LOGE(TAG, "Falling back to AP mode");
      nvs_erase_key(nvs, "ssid");
      nvs_erase_key(nvs, "pass");
      nvs_commit(nvs);
      esp_restart();
    }
    ip = WiFi.localIP();
  } else {
    // no ssid, ap mode
    ESP_LOGI(TAG, "No ssid and pass found in NVS, Initializing AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ECal");
//    WiFi.waitStatusBits(AP_STARTED_BIT, 10000);
    delay(500);
    ip = WiFi.softAPIP();
  }
  ESP_LOGI(TAG, "IP Address: %s", ip.toString().c_str());
}