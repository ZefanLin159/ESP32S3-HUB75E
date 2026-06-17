#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PLAY_MODE_LOOP_ALL = 0, /* default */
    PLAY_MODE_LOOP_ONE,
    PLAY_MODE_RANDOM,
    PLAY_MODE_SEQUENTIAL,
} audio_play_mode_t;

/**
 * @brief Initialize the audio agent and start its decoding task.
 */
esp_err_t audio_agent_init(void);

/**
 * @brief Set playback mode.
 */
esp_err_t audio_agent_set_mode(audio_play_mode_t mode);

/**
 * @brief Get current playback mode.
 */
audio_play_mode_t audio_agent_get_mode(void);

/**
 * @brief Adjust volume by a step (positive or negative percent).
 */
esp_err_t audio_agent_adjust_volume(int delta_percent);

/**
 * @brief Get current volume (0-100).
 */
uint8_t audio_agent_get_volume(void);

#ifdef __cplusplus
}
#endif
