#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Image scaling fit modes.
 */
typedef enum {
    IMG_FIT_STRETCH = 0,  /**< Directly stretch source to 64x64. */
    IMG_FIT_COVER,        /**< Scale uniformly, crop to fill 64x64. */
    IMG_FIT_CONTAIN,      /**< Scale uniformly, letterbox with black. */
} image_fit_t;

/**
 * @brief Load an image file and scale it to the requested RGB565 size.
 *
 * Supported formats: BMP (24/32-bit). JPG/PNG/GIF are reserved stubs.
 *
 * @param path      Absolute file path on the mounted filesystem.
 * @param out_fb    Caller-provided RGB565 framebuffer (out_w * out_h pixels).
 * @param out_w     Target width (e.g. 64).
 * @param out_h     Target height (e.g. 64).
 * @param fit       Scaling/fit mode.
 * @return ESP_OK on success, error otherwise.
 */
esp_err_t image_load_to_rgb565(const char *path,
                                uint16_t *out_fb,
                                int out_w, int out_h,
                                image_fit_t fit);

/**
 * @brief Return true if the file is an animated image (e.g. GIF).
 */
bool image_is_animated(const char *path);

/**
 * @brief Load a specific frame from an animated image.
 *
 * For non-animated formats frame_index is ignored.
 */
esp_err_t image_load_frame_to_rgb565(const char *path,
                                      int frame_index,
                                      uint16_t *out_fb,
                                      int out_w, int out_h,
                                      image_fit_t fit);

/**
 * @brief Return the number of frames in an animated image.
 */
int image_frame_count(const char *path);

/**
 * @brief Return the delay in milliseconds before showing the next frame.
 */
int image_frame_delay_ms(const char *path, int frame_index);

#ifdef __cplusplus
}
#endif
