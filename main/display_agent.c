#include "display_agent.h"
#include "message_broker.h"
#include "hub75e_driver.h"
#include "spectrum_analyzer.h"
#include "ui_animations.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define DISPLAY_TASK_STACK_SIZE (4 * 1024)
#define DISPLAY_TASK_PRIORITY   10
#define DISPLAY_PERIOD_MS       (1000 / CONFIG_PMC_DISPLAY_FPS)
#define BOOT_ANIM_DURATION_MS   5000

static const char *TAG = "display_agent";

static volatile display_mode_t s_mode = DISPLAY_MODE_BOOT;
static volatile bool s_needs_redraw = true;
static volatile uint32_t s_boot_start_ms = 0;

static void render_music_screen(void)
{
    uint16_t *fb = hub75e_driver_get_framebuffer();
    if (fb == NULL) {
        return;
    }

    ui_clear(fb, HUB75E_WIDTH, HUB75E_HEIGHT, 0);

    const float *bins = spectrum_get_bins();
    int bin_count = spectrum_get_bin_count();

    ui_color_t bar_color = ui_rgb(0, 200, 255);
    ui_draw_spectrum_bars(fb, HUB75E_WIDTH, HUB75E_HEIGHT - 16,
                          bins, bin_count, bar_color);
}

static void render_boot_screen(void)
{
    uint16_t *fb = hub75e_driver_get_framebuffer();
    if (fb == NULL) {
        return;
    }
    uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t boot_ms = now_ms - s_boot_start_ms;
    boot_animation_t anim = (boot_animation_t)((boot_ms / BOOT_ANIM_DURATION_MS) % BOOT_ANIM_COUNT);
    uint32_t anim_ms = boot_ms % BOOT_ANIM_DURATION_MS;

    ui_draw_boot_animation(fb, HUB75E_WIDTH, HUB75E_HEIGHT, anim, anim_ms);
}

static void render_photo_screen(void)
{
    uint16_t *fb = hub75e_driver_get_framebuffer();
    if (fb == NULL) {
        return;
    }
    ui_clear(fb, HUB75E_WIDTH, HUB75E_HEIGHT, ui_rgb(20, 20, 20));
    ui_draw_rect(fb, HUB75E_WIDTH, HUB75E_HEIGHT,
                 16, 16, HUB75E_WIDTH - 32, HUB75E_HEIGHT - 32,
                 ui_rgb(100, 255, 100));
}

static void render_settings_screen(void)
{
    uint16_t *fb = hub75e_driver_get_framebuffer();
    if (fb == NULL) {
        return;
    }
    ui_clear(fb, HUB75E_WIDTH, HUB75E_HEIGHT, ui_rgb(10, 10, 30));
    ui_draw_rect(fb, HUB75E_WIDTH, HUB75E_HEIGHT,
                 20, 20, 24, 24, ui_rgb(255, 255, 0));
}

static void render(void)
{
    switch (s_mode) {
    case DISPLAY_MODE_BOOT:
        render_boot_screen();
        break;
    case DISPLAY_MODE_MUSIC:
        render_music_screen();
        break;
    case DISPLAY_MODE_PHOTO:
        render_photo_screen();
        break;
    case DISPLAY_MODE_SETTINGS:
        render_settings_screen();
        break;
    default:
        break;
    }
}

static void display_task(void *arg)
{
    (void)arg;

    while (1) {
        message_t msg;
        if (receive_message(&msg, pdMS_TO_TICKS(DISPLAY_PERIOD_MS)) == ESP_OK) {
            if (msg.type == MSG_NEW_SPECTRUM_DATA && s_mode == DISPLAY_MODE_MUSIC) {
                s_needs_redraw = true;
            } else if (msg.type == MSG_DISPLAY_UPDATE) {
                s_needs_redraw = true;
            }
        }

        if (s_needs_redraw || s_mode == DISPLAY_MODE_BOOT) {
            render();
            s_needs_redraw = false;
        }

        hub75e_driver_refresh();
    }
}

esp_err_t display_agent_init(void)
{
    esp_err_t ret = hub75e_driver_init();
    if (ret != ESP_OK) {
        return ret;
    }

    s_boot_start_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    render_boot_screen();

    BaseType_t task = xTaskCreatePinnedToCore(
        display_task,
        "display_agent",
        DISPLAY_TASK_STACK_SIZE,
        NULL,
        DISPLAY_TASK_PRIORITY,
        NULL,
        1
    );
    if (task != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Display agent initialized");
    return ESP_OK;
}

esp_err_t display_agent_set_mode(display_mode_t mode)
{
    if (mode > DISPLAY_MODE_SETTINGS) {
        return ESP_ERR_INVALID_ARG;
    }
    s_mode = mode;
    s_needs_redraw = true;
    if (mode == DISPLAY_MODE_BOOT) {
        s_boot_start_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
    ESP_LOGI(TAG, "Display mode changed to %d", (int)mode);
    return ESP_OK;
}

esp_err_t display_agent_request_update(void)
{
    s_needs_redraw = true;
    return ESP_OK;
}
