#include "init.h"

#include "driver/rtc_io.h"
#include <FS.h>
#include <FSImpl.h>
#include <LittleFS.h>
#include <PAJ7620U2.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

static const char *TAG = "ECal Init";

nvs_handle_t nvs;

void einit() {
  ESP_LOGW(TAG, "ESP32 Booting");
  // Log memory and flash infos
  ESP_LOGI(TAG, "Chip Model: %s", ESP.getChipModel());
  ESP_LOGI(TAG, "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGI(TAG, "Minimum free heap: %d", ESP.getMinFreeHeap());
  ESP_LOGI(TAG, "Free PSRAM: %d", ESP.getFreePsram());
  ESP_LOGI(TAG, "Minimum free PSRAM: %d", ESP.getMinFreePsram());

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  if (!LittleFS.begin(true)) {
    ESP_LOGW(TAG, "LittleFS Mount Failed");
    esp_restart();
  }

  ESP_LOGI(TAG, "Opening NVS");
  ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs));
  int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
  error_t err = nvs_get_i32(nvs, "restart_counter", &restart_counter);
  NVS_ERROR_CHECK(err);
  ESP_LOGW(TAG, "Restarted %d times", restart_counter);
  restart_counter += 1;
  ESP_ERROR_CHECK(nvs_set_i32(nvs, "restart_counter", restart_counter));
}

std::string nvs_read_string(const char *key) {
  size_t required_size;
  error_t err = nvs_get_str(nvs, key, nullptr, &required_size);
  if (!NVS_ERROR_CHECK(err)) {
    return "";
  }
  char *value = (char *)malloc(required_size);
  err = nvs_get_str(nvs, key, value, &required_size);
  NVS_ERROR_CHECK(err);
  if (!NVS_ERROR_CHECK(err)) {
    return "";
  }
  std::string result = std::string(value);
  free(value);
  return result;
}

std::string format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[1024];
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  return {buf};
}