#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct audio_decoder audio_decoder_t;

/**
 * @brief Open an audio file and return a decoder instance.
 *
 * @param path Full path to the audio file.
 * @return Decoder handle, or NULL on failure.
 */
audio_decoder_t *audio_decoder_open(const char *path);

/**
 * @brief Close the decoder and free resources.
 */
void audio_decoder_close(audio_decoder_t *decoder);

/**
 * @brief Read interleaved 16-bit PCM frames.
 *
 * @param decoder Decoder handle.
 * @param samples Output buffer for 2 * frames int16_t samples.
 * @param frames  Number of stereo frames to read.
 * @return Number of frames actually read (0 on end of file / error).
 */
size_t audio_decoder_read_pcm(audio_decoder_t *decoder, int16_t *samples, size_t frames);

/**
 * @brief Get sample rate of the opened file.
 */
uint32_t audio_decoder_get_sample_rate(const audio_decoder_t *decoder);

/**
 * @brief Get number of channels (1 or 2).
 */
int audio_decoder_get_channels(const audio_decoder_t *decoder);

/**
 * @brief Check if decoder is still valid / has more data.
 */
bool audio_decoder_is_active(const audio_decoder_t *decoder);

#ifdef __cplusplus
}
#endif
