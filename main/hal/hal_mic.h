#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the I2S microphone interface.
 *
 * Uses a dedicated I2S port to avoid conflict with audio output.
 */
esp_err_t hal_mic_init(void);

/**
 * @brief Read PCM samples from the microphone.
 *
 * @param samples    Buffer to receive 16-bit PCM samples.
 * @param count      Number of samples to read.
 * @param read_count Actual number of samples read (may be 0 on timeout).
 * @return ESP_OK on success.
 */
esp_err_t hal_mic_read(int16_t *samples, size_t count, size_t *read_count);

#ifdef __cplusplus
}
#endif
