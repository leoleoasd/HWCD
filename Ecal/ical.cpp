#include "ical.h"
#include "init.h"
#include "url.hpp"
#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include <LittleFS.h>

const char * TAG = "ECal ical";



void download(){
  Url url(nvs_read_string("ical_url"));
//  std::string port = url.port();
//  if (port.empty()) {
//    if(url.scheme() == "http") {
//      port = "80";
//    } else if(url.scheme() == "https") {
//      port = "443";
//    }
//  }
  ESP_LOGI(TAG, "Downloading ICal from %s", url.str().c_str());
  WiFiClient wclient;
//  if (!wclient.connect("172.21.14.218", 7890)) {
//    ESP_LOGE(TAG, "Connection failed");
//    esp_restart();
//  }
//  ESP_LOGI(TAG, "Connected to proxy");
//  ESP_LOGI(TAG, "Proxy Command: %s", format("CONNECT %s:%s HTTP/1.0\r\n", url.host().c_str(), port.c_str()).c_str());
//  wclient.println(format("CONNECT %s:%s HTTP/1.0\r\n", url.host().c_str(), port.c_str()).c_str());
//  // two \r\n s
//  ESP_LOGI(TAG, "Got from proxy: %s", wclient.readStringUntil('\n').c_str());
//  wclient.readStringUntil('\n');
  HTTPClient client;
  if(!client.begin(wclient, url.str().c_str())) {
    ESP_LOGE(TAG, "Could not begin http wclient");
    esp_restart();
  }
  if(client.GET() != HTTP_CODE_OK) {
    ESP_LOGE(TAG, "Could not GET ical");
    ESP_LOGI(TAG, "Got ical: %s", client.getString().c_str());
    esp_restart();
  }
  ESP_LOGE(TAG, "ICal 200 OK");
  String ical = client.getString();
  ESP_LOGI(TAG, "Got ical: %s", ical.c_str());

  LittleFS.remove("/ical.ics");
  File f = LittleFS.open("/ical.ics", "w");
  if (!f) {
      ESP_LOGE(TAG, "Failed to open file for writing");
      esp_restart();
  }
  f.print(ical);
  f.close();
  ESP_LOGI(TAG, "Wrote ical to file");
}