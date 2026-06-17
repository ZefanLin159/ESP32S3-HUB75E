#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the spectrum analyzer.
 */
void spectrum_init(void);

/**
 * @brief Feed a chunk of interleaved 16-bit PCM samples.
 *
 * @param samples  Pointer to PCM samples.
 * @param frames   Number of frames (stereo pairs).
 * @param channels Number of channels (1 or 2).
 */
void spectrum_feed_pcm(const int16_t *samples, size_t frames, int channels);

/**
 * @brief Get the current spectrum magnitudes.
 *
 * @return Pointer to an array of CONFIG_SPECTRUM_NUM_BINS floats in [0,1].
 */
const float *spectrum_get_bins(void);

/**
 * @brief Get number of spectrum bins.
 */
int spectrum_get_bin_count(void);

#ifdef __cplusplus
}
#endif
