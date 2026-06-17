#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HUB75E_WIDTH  CONFIG_HUB75E_WIDTH
#define HUB75E_HEIGHT CONFIG_HUB75E_HEIGHT

/**
 * @brief Initialize HUB75E GPIO and frame buffer.
 */
esp_err_t hub75e_driver_init(void);

/**
 * @brief Clear the frame buffer with a color.
 */
void hub75e_driver_clear(uint16_t color);

/**
 * @brief Set a pixel in the frame buffer.
 */
void hub75e_driver_set_pixel(int x, int y, uint16_t color);

/**
 * @brief Get direct access to the frame buffer.
 */
uint16_t *hub75e_driver_get_framebuffer(void);

/**
 * @brief Refresh the physical panel from the frame buffer.
 *
 * Phase 1 stub: copies buffer and logs FPS. A full PIO/DMA driver
 * will replace this later.
 */
void hub75e_driver_refresh(void);

/**
 * @brief Set display brightness (0 - 100).
 */
void hub75e_driver_set_brightness(uint8_t brightness);

/**
 * @brief Fill the screen with a solid color for the specified duration.
 *
 * Useful as a startup test pattern to verify the panel and wiring.
 */
void hub75e_driver_show_test_pattern(uint32_t duration_ms, uint16_t color);

#ifdef __cplusplus
}
#endif
