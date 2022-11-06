#include "ical.h"
#include "init.h"
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
  ESP_LOGI(TAG, "Downloading ICal from %s",
           nvs_read_string("ical_url").c_str());
  WiFiClient wclient;
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
