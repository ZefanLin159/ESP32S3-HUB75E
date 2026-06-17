#include "audio_decoder_internal.h"
#include "wav_decoder.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_log.h"

static const char *TAG = "wav_dec";

#pragma pack(push, 1)
typedef struct {
    char riff[4];
    uint32_t file_size;
    char wave[4];
} wav_riff_header_t;

typedef struct {
    char fmt_id[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} wav_fmt_chunk_t;
#pragma pack(pop)

typedef struct {
    FILE *file;
    uint32_t data_start;
    uint32_t data_length;
    uint32_t sample_rate;
    uint16_t channels;
    uint16_t bits_per_sample;
    bool active;
} wav_context_t;

static size_t wav_read_pcm(void *ctx, int16_t *samples, size_t frames)
{
    wav_context_t *wav = (wav_context_t *)ctx;
    if (wav == NULL || wav->file == NULL || !wav->active) {
        return 0;
    }

    size_t frames_needed = frames;
    size_t frames_read = 0;

    while (frames_read < frames_needed) {
        int16_t left = 0, right = 0;

        if (wav->channels == 1) {
            if (fread(&left, sizeof(int16_t), 1, wav->file) != 1) {
                break;
            }
            right = left;
        } else {
            int16_t stereo[2];
            if (fread(stereo, sizeof(int16_t), 2, wav->file) != 2) {
                break;
            }
            left = stereo[0];
            right = stereo[1];
        }

        samples[frames_read * 2 + 0] = left;
        samples[frames_read * 2 + 1] = right;
        frames_read++;
    }

    if (frames_read == 0) {
        wav->active = false;
    }

    return frames_read;
}

static void wav_close(void *ctx)
{
    wav_context_t *wav = (wav_context_t *)ctx;
    if (wav == NULL) {
        return;
    }
    if (wav->file != NULL) {
        fclose(wav->file);
    }
    free(wav);
}

static uint32_t wav_sample_rate(void *ctx)
{
    wav_context_t *wav = (wav_context_t *)ctx;
    return wav ? wav->sample_rate : 0;
}

static int wav_channels(void *ctx)
{
    wav_context_t *wav = (wav_context_t *)ctx;
    return wav ? wav->channels : 0;
}

static bool wav_active(void *ctx)
{
    wav_context_t *wav = (wav_context_t *)ctx;
    return wav ? wav->active : false;
}

static const audio_decoder_ops_t s_wav_ops = {
    .read_pcm = wav_read_pcm,
    .close = wav_close,
    .sample_rate = wav_sample_rate,
    .channels = wav_channels,
    .active = wav_active,
};

audio_decoder_t *wav_decoder_open(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open %s", path);
        return NULL;
    }

    wav_riff_header_t riff;
    if (fread(&riff, sizeof(riff), 1, file) != 1) {
        ESP_LOGE(TAG, "Failed to read RIFF header");
        fclose(file);
        return NULL;
    }

    if (memcmp(riff.riff, "RIFF", 4) != 0 || memcmp(riff.wave, "WAVE", 4) != 0) {
        ESP_LOGE(TAG, "Invalid WAV header");
        fclose(file);
        return NULL;
    }

    wav_fmt_chunk_t fmt;
    bool fmt_found = false;
    uint32_t data_start = 0;
    uint32_t data_length = 0;

    while (!feof(file)) {
        char chunk_id[4];
        uint32_t chunk_size;

        if (fread(chunk_id, 1, 4, file) != 4) {
            break;
        }
        if (fread(&chunk_size, sizeof(chunk_size), 1, file) != 1) {
            break;
        }

        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            if (chunk_size < sizeof(fmt)) {
                ESP_LOGE(TAG, "fmt chunk too small");
                fclose(file);
                return NULL;
            }
            if (fread(&fmt, sizeof(fmt), 1, file) != 1) {
                ESP_LOGE(TAG, "Failed to read fmt chunk");
                fclose(file);
                return NULL;
            }
            if (chunk_size > sizeof(fmt)) {
                fseek(file, chunk_size - sizeof(fmt), SEEK_CUR);
            }
            fmt_found = true;
        } else if (memcmp(chunk_id, "data", 4) == 0) {
            data_start = (uint32_t)ftell(file);
            data_length = chunk_size;
            break;
        } else {
            fseek(file, chunk_size, SEEK_CUR);
        }
    }

    if (!fmt_found) {
        ESP_LOGE(TAG, "fmt chunk not found");
        fclose(file);
        return NULL;
    }

    if (fmt.audio_format != 1) {
        ESP_LOGE(TAG, "Unsupported WAV format (must be PCM)");
        fclose(file);
        return NULL;
    }

    if (fmt.bits_per_sample != 16) {
        ESP_LOGE(TAG, "Unsupported bit depth %d (need 16)", fmt.bits_per_sample);
        fclose(file);
        return NULL;
    }

    wav_context_t *ctx = calloc(1, sizeof(wav_context_t));
    if (ctx == NULL) {
        fclose(file);
        return NULL;
    }

    ctx->file = file;
    ctx->data_start = data_start;
    ctx->data_length = data_length;
    ctx->sample_rate = fmt.sample_rate;
    ctx->channels = fmt.num_channels;
    ctx->bits_per_sample = fmt.bits_per_sample;
    ctx->active = true;

    audio_decoder_t *dec = calloc(1, sizeof(audio_decoder_t));
    if (dec == NULL) {
        free(ctx);
        fclose(file);
        return NULL;
    }

    dec->ops = &s_wav_ops;
    dec->ctx = ctx;

    ESP_LOGI(TAG, "Opened WAV: sr=%lu ch=%u bits=%u data=%lu",
             ctx->sample_rate, ctx->channels, ctx->bits_per_sample, ctx->data_length);
    return dec;
}
