#include "spectrum_analyzer.h"
#include "sdkconfig.h"

#include <math.h>
#include <string.h>
#include "esp_timer.h"

#define NUM_BINS CONFIG_SPECTRUM_NUM_BINS
#define SMOOTH (CONFIG_SPECTRUM_SMOOTHING / 100.0f)

static float s_bins[NUM_BINS];
static float s_phase = 0.0f;

void spectrum_init(void)
{
    memset(s_bins, 0, sizeof(s_bins));
    s_phase = 0.0f;
}

void spectrum_feed_pcm(const int16_t *samples, size_t frames, int channels)
{
    if (samples == NULL || frames == 0) {
        return;
    }

    float energy = 0.0f;
    for (size_t i = 0; i < frames; i++) {
        int16_t left = samples[i * channels];
        int16_t right = (channels > 1) ? samples[i * channels + 1] : left;
        float l = left / 32768.0f;
        float r = right / 32768.0f;
        energy += (l * l + r * r);
    }

    float rms = sqrtf(energy / frames);
    if (rms > 1.0f) {
        rms = 1.0f;
    }

    /* Phase-driven animated distribution (placeholder until real FFT). */
    s_phase += 0.15f + rms * 0.3f;
    if (s_phase > 2.0f * M_PI) {
        s_phase -= 2.0f * M_PI;
    }

    for (int i = 0; i < NUM_BINS; i++) {
        float target = rms * (0.4f + 0.6f * fabsf(sinf(s_phase + i * 0.4f)));
        s_bins[i] = SMOOTH * s_bins[i] + (1.0f - SMOOTH) * target;
        if (s_bins[i] > 1.0f) {
            s_bins[i] = 1.0f;
        }
    }
}

const float *spectrum_get_bins(void)
{
    return s_bins;
}

int spectrum_get_bin_count(void)
{
    return NUM_BINS;
}
