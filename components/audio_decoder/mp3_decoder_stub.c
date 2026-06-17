#include "audio_decoder_internal.h"
#include "mp3_decoder_stub.h"
#include "esp_log.h"

static const char *TAG = "mp3_stub";

audio_decoder_t *mp3_decoder_stub_open(const char *path)
{
    (void)path;
    ESP_LOGW(TAG, "MP3 decoder is a stub; real MP3 support will be integrated later");
    return NULL;
}
