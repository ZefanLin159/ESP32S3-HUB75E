#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DISPLAY_MODE_BOOT = 0,
    DISPLAY_MODE_MUSIC,
    DISPLAY_MODE_PHOTO,
    DISPLAY_MODE_SETTINGS,
} display_mode_t;

/**
 * @brief Initialize the display agent and start the refresh task.
 */
esp_err_t display_agent_init(void);

/**
 * @brief Set the current display mode.
 */
esp_err_t display_agent_set_mode(display_mode_t mode);

/**
 * @brief Trigger a display update (e.g. new spectrum data ready).
 */
esp_err_t display_agent_request_update(void);

#ifdef __cplusplus
}
#endif
