#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure wake-up sources (any button) and power management.
 */
esp_err_t hal_power_init(void);

/**
 * @brief Enter deep sleep immediately. Wakes on any configured button press.
 */
esp_err_t hal_power_enter_deep_sleep(void);

#ifdef __cplusplus
}
#endif
