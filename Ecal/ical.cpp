#include "ical.h"
#include "init.h"
#include "url.hpp"
#include <FS.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <fstream>
#include <iostream>

using std::cout;

static const char *TAG = "ECal ical";

unique_ptr<ICalendar> calendar;

void download() {
  //  Url url(nvs_read_string("ical_url"));
  //  std::string port = url.port();
  //  if (port.empty()) {
  //    if(url.scheme() == "http") {
  //      port = "80";
  //    } else if(url.scheme() == "https") {
  //      port = "443";
  //    }
  //  }
  ESP_LOGI(TAG, "Downloading ICal from %s",
           nvs_read_string("ical_url").c_str());
  WiFiClient wclient;
  //  if (!wclient.connect("172.21.14.218", 7890)) {
  //    ESP_LOGE(TAG, "Connection failed");
  //    esp_restart();
  //  }
  //  ESP_LOGI(TAG, "Connected to proxy");
  //  ESP_LOGI(TAG, "Proxy Command: %s", format("CONNECT %s:%s HTTP/1.0\r\n",
  //  url.host().c_str(), port.c_str()).c_str());
  //  wclient.println(format("CONNECT %s:%s HTTP/1.0\r\n", url.host().c_str(),
  //  port.c_str()).c_str());
  //  // two \r\n s
  //  ESP_LOGI(TAG, "Got from proxy: %s",
  //  wclient.readStringUntil('\n').c_str()); wclient.readStringUntil('\n');
  // http://172.21.14.177:8000/esp32%20test.ics
  HTTPClient client;
  if (!client.begin(wclient, nvs_read_string("ical_url").c_str())) {
    ESP_LOGE(TAG, "Could not begin http wclient");
    esp_restart();
  }
  if (client.GET() != HTTP_CODE_OK) {
    ESP_LOGE(TAG, "Could not GET ical");
    ESP_LOGI(TAG, "Got ical: %s", client.getString().c_str());
    esp_restart();
  }
  ESP_LOGE(TAG, "ICal 200 OK");
  String ical = client.getString();
  ESP_LOGI(TAG, "Got ical");

  LittleFS.remove("/ical.ics");
  std::ofstream ical_file("/littlefs/ical.ics");
  if (!ical_file) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    esp_restart();
  }
  ical_file << ical.c_str();
  ical_file.close();
  ESP_LOGI(TAG, "Wrote ical to file");
  client.end();
}

void read() {
  calendar = unique_ptr<ICalendar>(new ICalendar("/littlefs/ical.ics"));
}
