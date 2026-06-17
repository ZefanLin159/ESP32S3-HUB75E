#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the three front-panel buttons.
 *
 * Sets up GPIOs with internal pull-ups and starts the button scan task.
 */
esp_err_t hal_buttons_init(void);

#ifdef __cplusplus
}
#endif
