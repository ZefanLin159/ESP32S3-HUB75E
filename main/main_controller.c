#include "main_controller.h"
#include "message_broker.h"
#include "audio_agent.h"
#include "display_agent.h"
#include "storage_agent.h"
#include "hal/hal_power.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define CONTROLLER_TASK_STACK_SIZE (4 * 1024)
#define CONTROLLER_TASK_PRIORITY    8

typedef enum {
    STATE_BOOT = 0,
    STATE_MUSIC,
    STATE_PHOTO,
    STATE_SETTINGS,
} system_state_t;

static const char *TAG = "main_ctrl";
static system_state_t s_state = STATE_BOOT;

static void set_state(system_state_t new_state)
{
    s_state = new_state;

    switch (s_state) {
    case STATE_BOOT:
        display_agent_set_mode(DISPLAY_MODE_BOOT);
        break;
    case STATE_MUSIC:
        display_agent_set_mode(DISPLAY_MODE_MUSIC);
        send_message(MSG_PLAY_PAUSE, 0, NULL, 0);
        break;
    case STATE_PHOTO:
        display_agent_set_mode(DISPLAY_MODE_PHOTO);
        send_message(MSG_PLAY_PAUSE, 0, NULL, 0); /* pause music in photo mode */
        break;
    case STATE_SETTINGS:
        display_agent_set_mode(DISPLAY_MODE_SETTINGS);
        break;
    default:
        break;
    }

    ESP_LOGI(TAG, "System state -> %d", (int)s_state);
}

static void cycle_mode(void)
{
    if (s_state == STATE_MUSIC) {
        set_state(STATE_PHOTO);
    } else if (s_state == STATE_PHOTO) {
        set_state(STATE_SETTINGS);
    } else {
        set_state(STATE_MUSIC);
    }
}

static void handle_message(const message_t *msg)
{
    switch (msg->type) {
    case MSG_STORAGE_SCAN_DONE:
        ESP_LOGI(TAG, "Storage scan complete, starting music mode");
        if (s_state == STATE_BOOT) {
            set_state(STATE_MUSIC);
        }
        break;

    case MSG_BUTTON_OK_SHORT:
        if (s_state == STATE_MUSIC) {
            send_message(MSG_PLAY_PAUSE, 0, NULL, 0);
        } else if (s_state == STATE_SETTINGS) {
            /* confirm setting */
            ESP_LOGI(TAG, "Setting confirmed");
        }
        break;

    case MSG_BUTTON_OK_LONG:
        hal_power_enter_deep_sleep();
        break;

    case MSG_BUTTON_LEFT_SHORT:
        if (s_state == STATE_MUSIC) {
            send_message(MSG_PLAY_PREV, 0, NULL, 0);
        } else if (s_state == STATE_SETTINGS) {
            ESP_LOGI(TAG, "Menu up");
        }
        break;

    case MSG_BUTTON_RIGHT_SHORT:
        if (s_state == STATE_MUSIC) {
            send_message(MSG_PLAY_NEXT, 0, NULL, 0);
        } else if (s_state == STATE_SETTINGS) {
            ESP_LOGI(TAG, "Menu down");
        }
        break;

    case MSG_BUTTON_LEFT_LONG:
        if (s_state == STATE_MUSIC) {
            send_message(MSG_VOLUME_DOWN, 0, NULL, 0);
            display_agent_request_update();
        }
        break;

    case MSG_BUTTON_RIGHT_LONG:
        if (s_state == STATE_MUSIC) {
            send_message(MSG_VOLUME_UP, 0, NULL, 0);
            display_agent_request_update();
        }
        break;

    case MSG_BUTTON_COMBO_MENU:
        ESP_LOGI(TAG, "Combo: menu");
        break;

    case MSG_BUTTON_COMBO_SETTINGS:
        if (s_state != STATE_SETTINGS) {
            set_state(STATE_SETTINGS);
        } else {
            set_state(STATE_MUSIC);
        }
        break;

    case MSG_MODE_SWITCH:
        cycle_mode();
        break;

    default:
        break;
    }
}

static void controller_task(void *arg)
{
    (void)arg;

    while (1) {
        message_t msg;
        if (receive_message(&msg, portMAX_DELAY) == ESP_OK) {
            handle_message(&msg);
        }
    }
}

esp_err_t main_controller_init(void)
{
    BaseType_t task = xTaskCreatePinnedToCore(
        controller_task,
        "main_ctrl",
        CONTROLLER_TASK_STACK_SIZE,
        NULL,
        CONTROLLER_TASK_PRIORITY,
        NULL,
        0
    );
    if (task != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Main controller initialized");
    return ESP_OK;
}
