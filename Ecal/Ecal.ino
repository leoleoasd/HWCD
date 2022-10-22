#include "Arduino.h"
#include "driver/rtc_io.h"
#include <PAJ7620U2.h>
#include <Wire.h>
#include <esp_sleep.h>


static const char *TAG = "ECal Main";

unsigned short Gesture_Data;
void setup() {
  ESP_LOGW(TAG, "ESP32 Booting");
  Wire.begin();
  delayMicroseconds(10);

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
  case ESP_SLEEP_WAKEUP_EXT0: {
    ESP_LOGV(TAG, "Wakeup caused by external signal using RTC_IO");
    Serial.print("gesture\n");
    Gesture_Data = I2C_readU16(PAJ_INT_FLAG1);
    if (Gesture_Data) {
      switch (Gesture_Data) {
      case PAJ_UP:
        Serial.print("Up\r\n");
        break;
      case PAJ_DOWN:
        Serial.print("Down\r\n");
        break;
      case PAJ_LEFT:
        Serial.print("Left\r\n");
        break;
      case PAJ_RIGHT:
        Serial.print("Right\r\n");
        break;
      case PAJ_FORWARD:
        Serial.print("Forward\r\n");
        break;
      case PAJ_BACKWARD:
        Serial.print("Backward\r\n");
        break;
      case PAJ_CLOCKWISE:
        Serial.print("Clockwise\r\n");
        break;
      case PAJ_COUNT_CLOCKWISE:
        Serial.print("AntiClockwise\r\n");
        break;
      case PAJ_WAVE:
        Serial.print("Wave\r\n");
        break;
      default:
        break;
      }
      Gesture_Data = 0;
    }
    break;
  }
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default: {
    uint8_t i;
    Serial.print("Gesture Sensor Test Program ...\n");
    delayMicroseconds(800);
    while (!PAJ7620U2_init()) {
      Serial.print("Gesture Sensor Error\n");
      delay(500);
    }
    Serial.print("Gesture Sensor OK\n");
    I2C_writeByte(PAJ_BANK_SELECT, 0); // Select Bank 0
    for (i = 0; i < Gesture_Array_SIZE; i++) {
      I2C_writeByte(Init_Gesture_Array[i][0],
                    Init_Gesture_Array[i][1]); // Gesture register initializes
    }
    break;
  }
  }
  rtc_gpio_pullup_en(GPIO_NUM_4);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
  delayMicroseconds(10);
  esp_deep_sleep_start();
}

int guesture = 0;

void loop() {
  if (guesture) {
  }
}

void ARDUINO_ISR_ATTR handle() { guesture = 1; }