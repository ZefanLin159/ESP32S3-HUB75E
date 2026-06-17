#pragma once

#include "audio_decoder.h"

/* Internal ops table implemented by each decoder backend (WAV, MP3, ...). */
typedef struct {
    size_t (*read_pcm)(void *ctx, int16_t *samples, size_t frames);
    void (*close)(void *ctx);
    uint32_t (*sample_rate)(void *ctx);
    int (*channels)(void *ctx);
    bool (*active)(void *ctx);
} audio_decoder_ops_t;

/* Concrete decoder handle: backend-specific context + its operations. */
struct audio_decoder {
    const audio_decoder_ops_t *ops;
    void *ctx;
};
