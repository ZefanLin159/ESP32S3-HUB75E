#include "display_agent.h"
#include "message_broker.h"
#include "hub75e_driver.h"
#include "spectrum_analyzer.h"
#include "ui_animations.h"
#include "image_processor.h"
#include "storage_agent.h"
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

/* Photo viewer state. */
static volatile int s_photo_index = 0;
static volatile int s_photo_frame_index = 0;
static volatile uint32_t s_photo_next_frame_ms = 0;

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

static void load_current_photo(uint16_t *fb)
{
    int count = storage_get_image_count();
    if (count <= 0) {
        ui_clear(fb, HUB75E_WIDTH, HUB75E_HEIGHT, ui_rgb(20, 20, 20));
        ui_draw_rect(fb, HUB75E_WIDTH, HUB75E_HEIGHT,
                     16, 28, HUB75E_WIDTH - 32, 8, ui_rgb(100, 255, 100));
        return;
    }

    if (s_photo_index < 0) {
        s_photo_index = count - 1;
    } else if (s_photo_index >= count) {
        s_photo_index = 0;
    }

    const char *path = storage_get_image_path(s_photo_index);
    if (path == NULL) {
        ui_clear(fb, HUB75E_WIDTH, HUB75E_HEIGHT, ui_rgb(20, 20, 20));
        return;
    }

    esp_err_t ret = image_load_frame_to_rgb565(path, s_photo_frame_index,
                                                fb, HUB75E_WIDTH, HUB75E_HEIGHT,
                                                IMG_FIT_COVER);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load image %s: %s", path, esp_err_to_name(ret));
        ui_clear(fb, HUB75E_WIDTH, HUB75E_HEIGHT, ui_rgb(20, 0, 0));
    }
}

static void render_photo_screen(void)
{
    uint16_t *fb = hub75e_driver_get_framebuffer();
    if (fb == NULL) {
        return;
    }
    load_current_photo(fb);
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
            } else if (msg.type == MSG_PHOTO_NEXT && s_mode == DISPLAY_MODE_PHOTO) {
                s_photo_index++;
                s_photo_frame_index = 0;
                s_photo_next_frame_ms = 0;
                s_needs_redraw = true;
            } else if (msg.type == MSG_PHOTO_PREV && s_mode == DISPLAY_MODE_PHOTO) {
                s_photo_index--;
                s_photo_frame_index = 0;
                s_photo_next_frame_ms = 0;
                s_needs_redraw = true;
            }
        }

        if (s_mode == DISPLAY_MODE_PHOTO) {
            uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (now_ms >= s_photo_next_frame_ms) {
                s_needs_redraw = true;
            }
        }

        if (s_needs_redraw || s_mode == DISPLAY_MODE_BOOT) {
            render();
            s_needs_redraw = false;

            if (s_mode == DISPLAY_MODE_PHOTO) {
                /* Advance GIF frame if applicable. */
                const char *path = storage_get_image_path(s_photo_index);
                if (path != NULL && image_is_animated(path)) {
                    int frame_count = image_frame_count(path);
                    s_photo_frame_index++;
                    if (s_photo_frame_index >= frame_count) {
                        s_photo_frame_index = 0;
                    }
                    uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    s_photo_next_frame_ms = now_ms + image_frame_delay_ms(path, s_photo_frame_index);
                }
            }
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
    } else if (mode == DISPLAY_MODE_PHOTO) {
        s_photo_index = 0;
        s_photo_frame_index = 0;
        s_photo_next_frame_ms = 0;
    }
    ESP_LOGI(TAG, "Display mode changed to %d", (int)mode);
    return ESP_OK;
}

esp_err_t display_agent_request_update(void)
{
    s_needs_redraw = true;
    return ESP_OK;
}
