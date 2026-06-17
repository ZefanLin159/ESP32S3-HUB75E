#include "hal_audio_output.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

#define AUDIO_I2S_PORT I2S_NUM_0

static const char *TAG = "hal_audio";
static i2s_chan_handle_t s_tx_chan = NULL;
static uint8_t s_volume = 70;

esp_err_t hal_audio_output_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(AUDIO_I2S_PORT, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;

    esp_err_t ret = i2s_new_channel(&chan_cfg, &s_tx_chan, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CONFIG_PMC_AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_PMC_I2S_BCLK_GPIO,
            .ws   = CONFIG_PMC_I2S_WS_GPIO,
            .dout = CONFIG_PMC_I2S_DOUT_GPIO,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    ret = i2s_channel_init_std_mode(s_tx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S std mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(s_tx_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Audio output initialized on I2S port %d", AUDIO_I2S_PORT);
    return ESP_OK;
}

esp_err_t hal_audio_output_set_volume(uint8_t volume)
{
    if (volume > 100) {
        volume = 100;
    }
    s_volume = volume;
    ESP_LOGI(TAG, "Volume set to %u%%", s_volume);
    return ESP_OK;
}

esp_err_t hal_audio_output_write(const int16_t *samples, size_t frames)
{
    if (s_tx_chan == NULL || samples == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (frames == 0) {
        return ESP_OK;
    }

    /* Apply volume scaling chunk by chunk to avoid modifying the caller's
     * buffer and to keep the scratch buffer out of large static RAM. */
    static int16_t s_scratch[256];
    const size_t max_frames = sizeof(s_scratch) / sizeof(s_scratch[0]) / 2;
    int32_t scale = s_volume;

    size_t frames_done = 0;
    while (frames_done < frames) {
        size_t chunk_frames = frames - frames_done;
        if (chunk_frames > max_frames) {
            chunk_frames = max_frames;
        }
        size_t chunk_samples = chunk_frames * 2;

        for (size_t i = 0; i < chunk_samples; i++) {
            int32_t v = ((int32_t)samples[frames_done * 2 + i] * scale) / 100;
            if (v > INT16_MAX) {
                v = INT16_MAX;
            } else if (v < INT16_MIN) {
                v = INT16_MIN;
            }
            s_scratch[i] = (int16_t)v;
        }

        size_t bytes_written = 0;
        size_t bytes_to_write = chunk_samples * sizeof(int16_t);
        esp_err_t ret = i2s_channel_write(s_tx_chan, s_scratch, bytes_to_write, &bytes_written, pdMS_TO_TICKS(500));
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "I2S write error: %s", esp_err_to_name(ret));
            return ret;
        }

        frames_done += chunk_frames;
    }

    return ESP_OK;
}
