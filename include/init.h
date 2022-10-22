//
// Created by Leo on 2022/10/22.
//

#ifndef HWCD_INCLUDE_INIT_H_
#define HWCD_INCLUDE_INIT_H_
#include "Arduino.h"
#include <Wire.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>

#define NVS_ERROR_CHECK(x)                                                     \
  ({                                                                           \
    esp_err_t err_rc_ = (x);                                                   \
    if (unlikely(err_rc_ != ESP_OK && err_rc_ != ESP_ERR_NVS_NOT_FOUND)) {     \
      _esp_error_check_failed(err_rc_, __FILE__, __LINE__, __ASSERT_FUNC, #x); \
    }                                                                          \
    err_rc_ == ESP_OK;                                                         \
  })

extern nvs_handle_t nvs;
void einit();
String nvs_read_string(const char *);

#endif // HWCD_INCLUDE_INIT_H_
