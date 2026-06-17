#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the I2S audio output (DAC / I2S amplifier).
 */
esp_err_t hal_audio_output_init(void);

/**
 * @brief Set output volume (0 - 100).
 */
esp_err_t hal_audio_output_set_volume(uint8_t volume);

/**
 * @brief Write interleaved 16-bit stereo PCM frames.
 *
 * @param frames   Number of stereo frames (left/right pairs).
 * @param samples  Pointer to 2 * frames int16_t samples.
 * @return ESP_OK on success.
 */
esp_err_t hal_audio_output_write(const int16_t *samples, size_t frames);

#ifdef __cplusplus
}
#endif
