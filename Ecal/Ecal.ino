#include "Arduino.h"
#include "driver/rtc_io.h"
#include <WiFi.h>
#include <Wire.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include "display.h"
#include "e_wifi.h"
#include "gesture.h"
#include "http.h"
#include "ical.h"
#include "init.h"
#include "menu.h"

static const char *TAG = "ECal Main";

void deepsleep_awake() __attribute__((noreturn));

void setup() {
  switch (esp_reset_reason()) {
  case ESP_RST_POWERON:
    break;
  case ESP_RST_DEEPSLEEP:
    einit();
    setenv("TZ", "CST-8", 1);
    init_menu();
    tzset();
    display_init();
    read();
    deepsleep_awake();
  default:
    ESP_LOGW(TAG, "Unexpected reset reason: %d", esp_reset_reason());
  }
  einit();
  wifi_init();
  http_init();
  if (WiFiClass::getMode() == WIFI_MODE_AP) {
    display_instruction(
        format("Connect to Ecal Wifi, then go to http://%s to configure",
               WiFi.softAPIP().toString().c_str()));
    return;
  }
  if (nvs_read_string("ical_url").empty()) {
    display_instruction(format("No Ical, go to http://%s to configure",
                               WiFi.localIP().toString().c_str()));
    return;
  }
  download();
  gesture_init();
  init_menu();
  display_init();
  display_clear();
  read();
  set_start_of_current_week();
  clear_buf();
  display_calendar();
  display_draw();

  ESP_LOGI(TAG, "main: entering deep sleep");
  rtc_gpio_pullup_en(GPIO_NUM_4);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
  esp_sleep_enable_timer_wakeup(60 * 15 * 1000000ULL); // 15 min
  delayMicroseconds(10);
  esp_deep_sleep_start();
}

void deepsleep_awake() {
  tm current_time;
  getLocalTime(&current_time);
  //  current_time.tm_hour = 12; // TODO: REMOVE
  ESP_LOGI(TAG, "Current time: %s", asctime(&current_time));
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
  case ESP_SLEEP_WAKEUP_EXT0: {
    ESP_LOGV(TAG, "Wakeup caused by external signal using RTC_IO");
    handle_gesture();
    break;
  }
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    set_start_of_current_week();
    clear_buf();
    display_calendar();
    display_draw();
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default: {
  }
  }
  ESP_LOGI(TAG, "ds: entering deep sleep");
  rtc_gpio_pullup_en(GPIO_NUM_4);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
  esp_sleep_enable_timer_wakeup(60 * 15 * 1000000ULL); // 15 min
  delayMicroseconds(10);
  esp_deep_sleep_start();
}

void loop() {
  //  if (guesture) {
  //  }
  delay(5000);
  ESP_LOGI(TAG, "loop");
}
