#include "display.h"
#include <Arduino.h>
#include <PAJ7620U2.h>
#include <gesture.h>
#include "menu.h"

static const char *TAG = "Ecal gesture";

RTC_SLOW_ATTR uint8_t mode = 0; // 0: calendar, 1: menu

void gesture_init() {
  uint8_t i;
  ESP_LOGI(TAG, "Initializing Gesture Sensor");
  Wire.begin();
  delayMicroseconds(10);
  while (!PAJ7620U2_init()) {
    ESP_LOGW(TAG, "Gesture Sensor Error");
    delay(1000);
  }
  ESP_LOGI(TAG, "Gesture Sensor Initialized");
  I2C_writeByte(PAJ_BANK_SELECT, 0); // Select Bank 0
  for (i = 0; i < Gesture_Array_SIZE; i++) {
    I2C_writeByte(Init_Gesture_Array[i][0],
                  Init_Gesture_Array[i][1]); // Gesture register
  }
}

void handle_gesture() {
  ESP_LOGI(TAG, "gesture");
  Wire.begin();
  delayMicroseconds(10);
  uint16_t Gesture_Data;
  Gesture_Data = I2C_readU16(PAJ_INT_FLAG1);
  if (Gesture_Data) {
    switch (Gesture_Data) {
    case PAJ_UP:
      ESP_LOGI(TAG, "UP");
      gesture_up();
      break;
    case PAJ_DOWN:
      ESP_LOGI(TAG, "DOWN");
      gesture_down();
      break;
    case PAJ_LEFT:
      ESP_LOGI(TAG, "LEFT");
      gesture_left();
      break;
    case PAJ_RIGHT:
      ESP_LOGI(TAG, "RIGHT");
      gesture_right();
      break;
    default:
      break;
    }
  }
}

void gesture_up() {
  mode = 1;
  menu_up();
}
void gesture_down() {
  mode = 1;
  menu_down();
}
void gesture_left() {
  if (mode == 0) {
    ESP_LOGI(TAG, "Gesture: Left");
    calendar_left();
    display_calendar();
  }
  if (mode == 1) {
    menu_leave();
  }
}
void gesture_right() {
  if (mode == 0) {
    ESP_LOGI(TAG, "Gesture: Right");
    calendar_right();
    display_calendar();
  }
  if (mode == 1) {
    menu_enter();
  }
}