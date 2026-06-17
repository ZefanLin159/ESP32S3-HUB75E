#include <stdio.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "message_broker.h"
#include "main_controller.h"
#include "audio_agent.h"
#include "display_agent.h"
#include "storage_agent.h"
#include "spectrum_analyzer.h"

#include "hub75e_driver.h"
#include "hal/hal_buttons.h"
#include "hal/hal_power.h"
#include "hal/hal_mic.h"
#include "hal/hal_audio_output.h"

static const char *TAG = "app_main";

void app_main(void)
{
    ESP_LOGI(TAG, "Pixel Music Companion starting...");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(message_broker_init());

    ESP_ERROR_CHECK(display_agent_init());

    /* Default to a full-white screen on power-up to verify panel/wiring. */
    hub75e_driver_clear(0xFFFF);
    hub75e_driver_refresh();

    ESP_LOGI(TAG, "Minimal init complete, entering idle loop");

    /* Idle loop. */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Heartbeat: free heap=%lu bytes", esp_get_free_heap_size());
    }
}
