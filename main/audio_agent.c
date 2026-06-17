#include "audio_agent.h"
#include "message_broker.h"
#include "storage_agent.h"
#include "audio_decoder.h"
#include "spectrum_analyzer.h"
#include "hal/hal_audio_output.h"
#include "sdkconfig.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#define AUDIO_TASK_STACK_SIZE   (8 * 1024)
#define AUDIO_TASK_PRIORITY     12
#define AUDIO_READ_FRAMES       512
#define VOLUME_STEP             5

static const char *TAG = "audio_agent";

static audio_play_mode_t s_mode = PLAY_MODE_LOOP_ALL;
static bool s_playing = false;
static bool s_paused = false;
static int s_current_index = 0;
static uint8_t s_volume = 70;
static audio_decoder_t *s_decoder = NULL;

static void advance_track(int direction)
{
    int count = storage_get_music_count();
    if (count <= 0) {
        s_current_index = 0;
        return;
    }

    if (s_mode == PLAY_MODE_RANDOM) {
        if (count == 1) {
            s_current_index = 0;
        } else {
            int next = s_current_index;
            while (next == s_current_index) {
                next = rand() % count;
            }
            s_current_index = next;
        }
    } else {
        s_current_index += direction;
        if (s_current_index < 0) {
            s_current_index = count - 1;
        } else if (s_current_index >= count) {
            s_current_index = 0;
        }
    }
}

static void load_and_play_current(void)
{
    if (s_decoder != NULL) {
        audio_decoder_close(s_decoder);
        s_decoder = NULL;
    }

    const char *path = storage_get_music_path(s_current_index);
    if (path == NULL) {
        ESP_LOGW(TAG, "No music file at index %d", s_current_index);
        s_playing = false;
        return;
    }

    s_decoder = audio_decoder_open(path);
    if (s_decoder == NULL) {
        ESP_LOGE(TAG, "Failed to open %s", path);
        s_playing = false;
        return;
    }

    ESP_LOGI(TAG, "Now playing: %s", path);
    send_message(MSG_AUDIO_TRACK_STARTED, s_current_index, NULL, 0);
    s_playing = true;
    s_paused = false;
}

static void handle_control_message(const message_t *msg)
{
    switch (msg->type) {
    case MSG_PLAY_PAUSE:
        if (s_playing) {
            s_paused = !s_paused;
            ESP_LOGI(TAG, "Playback %s", s_paused ? "paused" : "resumed");
        } else if (storage_get_music_count() > 0) {
            load_and_play_current();
        }
        break;

    case MSG_PLAY_NEXT:
        advance_track(1);
        load_and_play_current();
        break;

    case MSG_PLAY_PREV:
        advance_track(-1);
        load_and_play_current();
        break;

    case MSG_VOLUME_UP:
        audio_agent_adjust_volume(VOLUME_STEP);
        break;

    case MSG_VOLUME_DOWN:
        audio_agent_adjust_volume(-VOLUME_STEP);
        break;

    case MSG_MODE_SWITCH:
        audio_agent_set_mode((audio_play_mode_t)((s_mode + 1) % (PLAY_MODE_SEQUENTIAL + 1)));
        break;

    default:
        break;
    }
}

static void audio_task(void *arg)
{
    (void)arg;

    int16_t *pcm = heap_caps_malloc(AUDIO_READ_FRAMES * 2 * sizeof(int16_t),
                                    MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (pcm == NULL) {
        ESP_LOGE(TAG, "Failed to allocate PCM buffer");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        message_t msg;
        if (receive_message(&msg, pdMS_TO_TICKS(5)) == ESP_OK) {
            handle_control_message(&msg);
        }

        if (!s_playing || s_paused || s_decoder == NULL) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        size_t frames = audio_decoder_read_pcm(s_decoder, pcm, AUDIO_READ_FRAMES);
        if (frames > 0) {
            spectrum_feed_pcm(pcm, frames, audio_decoder_get_channels(s_decoder));
            hal_audio_output_write(pcm, frames);
            send_message(MSG_NEW_SPECTRUM_DATA, 0, NULL, 0);
        } else {
            /* Track ended */
            send_message(MSG_AUDIO_TRACK_FINISHED, s_current_index, NULL, 0);
            audio_decoder_close(s_decoder);
            s_decoder = NULL;

            if (s_mode == PLAY_MODE_LOOP_ONE) {
                load_and_play_current();
            } else if (s_mode == PLAY_MODE_SEQUENTIAL && s_current_index + 1 >= storage_get_music_count()) {
                s_playing = false;
                ESP_LOGI(TAG, "Sequential playback finished");
            } else {
                advance_track(1);
                load_and_play_current();
            }
        }
    }
}

esp_err_t audio_agent_init(void)
{
    srand((unsigned)xTaskGetTickCount());

    BaseType_t task = xTaskCreatePinnedToCore(
        audio_task,
        "audio_agent",
        AUDIO_TASK_STACK_SIZE,
        NULL,
        AUDIO_TASK_PRIORITY,
        NULL,
        1
    );

    if (task != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Audio agent initialized");
    return ESP_OK;
}

esp_err_t audio_agent_set_mode(audio_play_mode_t mode)
{
    if (mode > PLAY_MODE_SEQUENTIAL) {
        return ESP_ERR_INVALID_ARG;
    }
    s_mode = mode;
    ESP_LOGI(TAG, "Play mode changed to %d", (int)mode);
    return ESP_OK;
}

audio_play_mode_t audio_agent_get_mode(void)
{
    return s_mode;
}

esp_err_t audio_agent_adjust_volume(int delta_percent)
{
    int new_vol = (int)s_volume + delta_percent;
    if (new_vol < 0) {
        new_vol = 0;
    } else if (new_vol > 100) {
        new_vol = 100;
    }
    s_volume = (uint8_t)new_vol;
    hal_audio_output_set_volume(s_volume);
    return ESP_OK;
}

uint8_t audio_agent_get_volume(void)
{
    return s_volume;
}
