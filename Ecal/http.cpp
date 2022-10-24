#include "http.h"

#include "Arduino.h"
#include "init.h"
#include <ESPAsyncWebServer.h>

static const char *TAG = "Ecal http";

//const char index_html[] = R"html()html";
extern const char index_html[] asm("_binary_Ecal_html_index_html_start");

String processor(const String& var)
{
  size_t length;
  if (!NVS_ERROR_CHECK(nvs_get_str(nvs, var.c_str(), nullptr, &length))) {
    return "";
  }
  char *buffer = new char[length];
  if (!NVS_ERROR_CHECK(nvs_get_str(nvs, var.c_str(), buffer, &length))) {
    return "";
  };
  String ret = buffer;
  delete[] buffer;
  return ret;
}

void home(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", index_html, processor);
}

void update(AsyncWebServerRequest *request) {
  size_t params = request->params();
  for(size_t i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    ESP_ERROR_CHECK(nvs_set_str(nvs, p->name().c_str(), p->value().c_str()));
  }
  // Restart esp32 to use new wifi settings
  esp_restart();

  request->send_P(200, "text/html", index_html, processor);
}

void clear(AsyncWebServerRequest *request) {
  ESP_ERROR_CHECK(nvs_erase_all(nvs));
  esp_restart();
}

AsyncWebServer server(80);

void http_init() {
  ESP_LOGI(TAG, "Starting HTTP Server");
  server.on("/", HTTP_GET, home);
  server.on("/update", HTTP_GET, update);
  server.on("/clear", HTTP_GET, clear);
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "not Found!");
  });
  server.begin();
  ESP_LOGI(TAG, "Started HTTP Server");
}