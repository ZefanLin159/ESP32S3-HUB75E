#include "hal_buttons.h"
#include "message_broker.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define BUTTON_SCAN_PERIOD_MS 20

typedef enum {
    BUTTON_LEFT = 0,
    BUTTON_RIGHT,
    BUTTON_OK,
    BUTTON_COUNT
} button_id_t;

static const char *TAG = "hal_buttons";

static const gpio_num_t s_button_gpio[BUTTON_COUNT] = {
    [BUTTON_LEFT] = CONFIG_PMC_BUTTON_LEFT_GPIO,
    [BUTTON_RIGHT] = CONFIG_PMC_BUTTON_RIGHT_GPIO,
    [BUTTON_OK] = CONFIG_PMC_BUTTON_OK_GPIO,
};

typedef struct {
    bool pressed;          /* debounced state */
    bool long_sent;        /* long-press event already emitted */
    bool combo_sent;       /* combo event already emitted for this press */
    TickType_t press_tick; /* tick when debounced press started */
} button_state_t;

static button_state_t s_state[BUTTON_COUNT];

static inline bool is_pressed(button_id_t id)
{
    return gpio_get_level(s_button_gpio[id]) == 0;
}

static void emit_button_event(button_id_t id, bool is_long)
{
    switch (id) {
    case BUTTON_LEFT:
        send_message(is_long ? MSG_BUTTON_LEFT_LONG : MSG_BUTTON_LEFT_SHORT, 0, NULL, 0);
        break;
    case BUTTON_RIGHT:
        send_message(is_long ? MSG_BUTTON_RIGHT_LONG : MSG_BUTTON_RIGHT_SHORT, 0, NULL, 0);
        break;
    case BUTTON_OK:
        send_message(is_long ? MSG_BUTTON_OK_LONG : MSG_BUTTON_OK_SHORT, 0, NULL, 0);
        break;
    default:
        break;
    }
}

static void check_combinations(void)
{
    /* OK + LEFT => menu, OK + RIGHT => settings. Only emit once per OK hold. */
    if (!s_state[BUTTON_OK].pressed) {
        return;
    }

    if (s_state[BUTTON_LEFT].pressed && !s_state[BUTTON_LEFT].combo_sent) {
        send_message(MSG_BUTTON_COMBO_MENU, 0, NULL, 0);
        s_state[BUTTON_LEFT].combo_sent = true;
    }

    if (s_state[BUTTON_RIGHT].pressed && !s_state[BUTTON_RIGHT].combo_sent) {
        send_message(MSG_BUTTON_COMBO_SETTINGS, 0, NULL, 0);
        s_state[BUTTON_RIGHT].combo_sent = true;
    }
}

static void button_scan_task(void *arg)
{
    while (1) {
        TickType_t now = xTaskGetTickCount();

        for (int i = 0; i < BUTTON_COUNT; i++) {
            bool raw = is_pressed(i);

            if (raw && !s_state[i].pressed) {
                /* New press (after debounce - we assume scan period acts as debounce) */
                s_state[i].pressed = true;
                s_state[i].long_sent = false;
                s_state[i].combo_sent = false;
                s_state[i].press_tick = now;
            } else if (!raw && s_state[i].pressed) {
                /* Release */
                TickType_t duration = now - s_state[i].press_tick;
                uint32_t duration_ms = duration * portTICK_PERIOD_MS;

                bool was_long = false;
                if (i == BUTTON_OK) {
                    was_long = duration_ms >= CONFIG_PMC_BUTTON_SLEEP_PRESS_MS;
                } else {
                    was_long = duration_ms >= CONFIG_PMC_BUTTON_LONG_PRESS_MS;
                }

                if (!was_long) {
                    emit_button_event(i, false);
                }
                s_state[i].pressed = false;
            }
        }

        check_combinations();

        /* Long press detection while held */
        for (int i = 0; i < BUTTON_COUNT; i++) {
            if (!s_state[i].pressed || s_state[i].long_sent) {
                continue;
            }
            TickType_t duration = now - s_state[i].press_tick;
            uint32_t duration_ms = duration * portTICK_PERIOD_MS;

            if (i == BUTTON_OK) {
                if (duration_ms >= CONFIG_PMC_BUTTON_SLEEP_PRESS_MS) {
                    emit_button_event(i, true);
                    s_state[i].long_sent = true;
                }
            } else {
                if (duration_ms >= CONFIG_PMC_BUTTON_LONG_PRESS_MS) {
                    emit_button_event(i, true);
                    s_state[i].long_sent = true;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_SCAN_PERIOD_MS));
    }
}

esp_err_t hal_buttons_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 0,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };

    for (int i = 0; i < BUTTON_COUNT; i++) {
        io_conf.pin_bit_mask |= (1ULL << s_button_gpio[i]);
    }

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    BaseType_t task = xTaskCreatePinnedToCore(
        button_scan_task,
        "button_scan",
        2048,
        NULL,
        5,
        NULL,
        0
    );
    if (task != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Buttons initialized");
    return ESP_OK;
}
