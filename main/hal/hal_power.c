#include "hal_power.h"
#include "sdkconfig.h"

#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define BUTTON_MASK ((1ULL << CONFIG_PMC_BUTTON_LEFT_GPIO) | \
                     (1ULL << CONFIG_PMC_BUTTON_RIGHT_GPIO) | \
                     (1ULL << CONFIG_PMC_BUTTON_OK_GPIO))

static const char *TAG = "hal_power";

esp_err_t hal_power_init(void)
{
    /* Wake on any button going LOW (active-low with pull-up). */
    esp_err_t ret = esp_sleep_enable_ext1_wakeup(BUTTON_MASK, ESP_EXT1_WAKEUP_ANY_LOW);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure wake-up source: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Power management initialized, wake mask=0x%llx", BUTTON_MASK);
    return ESP_OK;
}

esp_err_t hal_power_enter_deep_sleep(void)
{
    ESP_LOGI(TAG, "Entering deep sleep now");
    esp_deep_sleep_start();
    return ESP_OK;
}
