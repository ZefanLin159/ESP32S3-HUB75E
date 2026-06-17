#include "hal_mic.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

#define MIC_I2S_PORT I2S_NUM_1

static const char *TAG = "hal_mic";
static i2s_chan_handle_t s_rx_chan = NULL;

esp_err_t hal_mic_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(MIC_I2S_PORT, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;

    esp_err_t ret = i2s_new_channel(&chan_cfg, NULL, &s_rx_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S RX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CONFIG_PMC_AUDIO_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = CONFIG_PMC_MIC_BCLK_GPIO,
            .ws   = CONFIG_PMC_MIC_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din  = CONFIG_PMC_I2S_DIN_GPIO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    ret = i2s_channel_init_std_mode(s_rx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S std mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2s_channel_enable(s_rx_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable I2S RX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Microphone initialized on I2S port %d", MIC_I2S_PORT);
    return ESP_OK;
}

esp_err_t hal_mic_read(int16_t *samples, size_t count, size_t *read_count)
{
    if (s_rx_chan == NULL || samples == NULL || read_count == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    size_t bytes_read = 0;
    size_t bytes_to_read = count * sizeof(int16_t);
    esp_err_t ret = i2s_channel_read(s_rx_chan, samples, bytes_to_read, &bytes_read, pdMS_TO_TICKS(100));
    if (ret != ESP_OK && ret != ESP_ERR_TIMEOUT) {
        return ret;
    }

    *read_count = bytes_read / sizeof(int16_t);
    return ESP_OK;
}
