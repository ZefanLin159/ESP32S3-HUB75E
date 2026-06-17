#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the main controller task.
 *
 * Coordinates button events, audio agent, display agent, and storage.
 */
esp_err_t main_controller_init(void);

#ifdef __cplusplus
}
#endif
