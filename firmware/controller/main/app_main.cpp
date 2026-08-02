#include "controller_runtime.hpp"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

namespace {

const char* reset_reason_name(const esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:
      return "POWER_ON";
    case ESP_RST_EXT:
      return "EXTERNAL";
    case ESP_RST_SW:
      return "SOFTWARE";
    case ESP_RST_PANIC:
      return "PANIC";
    case ESP_RST_INT_WDT:
      return "INT_WATCHDOG";
    case ESP_RST_TASK_WDT:
      return "TASK_WATCHDOG";
    case ESP_RST_WDT:
      return "OTHER_WATCHDOG";
    case ESP_RST_DEEPSLEEP:
      return "DEEP_SLEEP";
    case ESP_RST_BROWNOUT:
      return "BROWNOUT";
    case ESP_RST_SDIO:
      return "SDIO";
    default:
      return "UNKNOWN";
  }
}

void initialize_nvs() {
  const esp_err_t result = nvs_flash_init();
  if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW("CONFIG", "reinitializing NVS after %s", esp_err_to_name(result));
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  } else {
    ESP_ERROR_CHECK(result);
  }
}

}  // namespace

extern "C" void app_main(void) {
  const char* const reset_reason = reset_reason_name(esp_reset_reason());
  ESP_LOGI("APP", "Radiance3D native ESP-IDF startup; reset=%s", reset_reason);
  initialize_nvs();
  const bool initialized = radiance3d::controller_runtime_initialize(reset_reason);
  if (!initialized) {
    ESP_LOGE("APP", "physical controller initialization failed; outputs remain disabled");
  }
  radiance3d::controller_runtime_start();
}
