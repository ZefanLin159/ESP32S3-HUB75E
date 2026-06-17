#include "audio_decoder_internal.h"
#include "wav_decoder.h"
#include "mp3_decoder_stub.h"

#include <string.h>
#include <strings.h>
#include "esp_log.h"

static const char *TAG = "audio_decoder";

audio_decoder_t *audio_decoder_open(const char *path)
{
    if (path == NULL) {
        return NULL;
    }

    size_t len = strlen(path);
    if (len < 4) {
        return NULL;
    }

    const char *ext = path + len - 4;

    if (strcasecmp(ext, ".wav") == 0) {
        return wav_decoder_open(path);
    }

    if (strcasecmp(ext, ".mp3") == 0) {
        return mp3_decoder_stub_open(path);
    }

    ESP_LOGE(TAG, "Unsupported file extension: %s", ext);
    return NULL;
}

void audio_decoder_close(audio_decoder_t *decoder)
{
    if (decoder == NULL || decoder->ops == NULL) {
        return;
    }
    if (decoder->ops->close != NULL) {
        decoder->ops->close(decoder->ctx);
    }
    free(decoder);
}

size_t audio_decoder_read_pcm(audio_decoder_t *decoder, int16_t *samples, size_t frames)
{
    if (decoder == NULL || decoder->ops == NULL || decoder->ops->read_pcm == NULL) {
        return 0;
    }
    return decoder->ops->read_pcm(decoder->ctx, samples, frames);
}

uint32_t audio_decoder_get_sample_rate(const audio_decoder_t *decoder)
{
    if (decoder == NULL || decoder->ops == NULL || decoder->ops->sample_rate == NULL) {
        return 0;
    }
    return decoder->ops->sample_rate(decoder->ctx);
}

int audio_decoder_get_channels(const audio_decoder_t *decoder)
{
    if (decoder == NULL || decoder->ops == NULL || decoder->ops->channels == NULL) {
        return 0;
    }
    return decoder->ops->channels(decoder->ctx);
}

bool audio_decoder_is_active(const audio_decoder_t *decoder)
{
    if (decoder == NULL || decoder->ops == NULL || decoder->ops->active == NULL) {
        return false;
    }
    return decoder->ops->active(decoder->ctx);
}
