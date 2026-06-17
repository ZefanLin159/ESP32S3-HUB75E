#include "hub75e_driver.h"
#include "sdkconfig.h"

#include <string.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"

#define HUB75E_WIDTH  CONFIG_HUB75E_WIDTH
#define HUB75E_HEIGHT CONFIG_HUB75E_HEIGHT

static const char *TAG = "hub75e";

static MatrixPanel_I2S_DMA *s_dma_display = nullptr;
static uint16_t *s_framebuffer = NULL;
static volatile uint8_t s_brightness = 65;
static bool s_initialized = false;

static inline uint16_t rgb565(uint16_t color)
{
    /* Both the UI and the DMA library use the standard RGB565 format. */
    return color;
}

esp_err_t hub75e_driver_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    size_t fb_size = HUB75E_WIDTH * HUB75E_HEIGHT * sizeof(uint16_t);
    s_framebuffer = (uint16_t *)heap_caps_malloc(fb_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (s_framebuffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate frame buffer");
        return ESP_ERR_NO_MEM;
    }
    memset(s_framebuffer, 0, fb_size);

    HUB75_I2S_CFG::i2s_pins pins = {
        CONFIG_HUB75E_PIN_R1,
        CONFIG_HUB75E_PIN_G1,
        CONFIG_HUB75E_PIN_B1,
        CONFIG_HUB75E_PIN_R2,
        CONFIG_HUB75E_PIN_G2,
        CONFIG_HUB75E_PIN_B2,
        CONFIG_HUB75E_PIN_A,
        CONFIG_HUB75E_PIN_B,
        CONFIG_HUB75E_PIN_C,
        CONFIG_HUB75E_PIN_D,
        CONFIG_HUB75E_PIN_E,
        CONFIG_HUB75E_PIN_LAT,
        CONFIG_HUB75E_PIN_OE,
        CONFIG_HUB75E_PIN_CLK
    };

    /* Use the same defaults as the working reference project. */
    HUB75_I2S_CFG mxconfig(HUB75E_WIDTH, HUB75E_HEIGHT, 1);
    mxconfig.gpio = pins;

    s_dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    if (s_dma_display == nullptr) {
        heap_caps_free(s_framebuffer);
        s_framebuffer = NULL;
        ESP_LOGE(TAG, "Failed to create MatrixPanel_I2S_DMA");
        return ESP_ERR_NO_MEM;
    }

    if (!s_dma_display->begin()) {
        delete s_dma_display;
        s_dma_display = nullptr;
        heap_caps_free(s_framebuffer);
        s_framebuffer = NULL;
        ESP_LOGE(TAG, "MatrixPanel_I2S_DMA begin() failed");
        return ESP_FAIL;
    }

    s_dma_display->setBrightness8(s_brightness);
    s_dma_display->clearScreen();

    s_initialized = true;
    ESP_LOGI(TAG, "HUB75E DMA driver initialized: %dx%d", HUB75E_WIDTH, HUB75E_HEIGHT);
    return ESP_OK;
}

void hub75e_driver_clear(uint16_t color)
{
    if (!s_initialized || s_framebuffer == NULL) {
        return;
    }
    for (int i = 0; i < HUB75E_WIDTH * HUB75E_HEIGHT; i++) {
        s_framebuffer[i] = color;
    }
}

void hub75e_driver_set_pixel(int x, int y, uint16_t color)
{
    if (!s_initialized || s_framebuffer == NULL ||
        x < 0 || x >= HUB75E_WIDTH || y < 0 || y >= HUB75E_HEIGHT) {
        return;
    }
    s_framebuffer[y * HUB75E_WIDTH + x] = color;
}

uint16_t *hub75e_driver_get_framebuffer(void)
{
    return s_framebuffer;
}

void hub75e_driver_refresh(void)
{
    if (!s_initialized || s_dma_display == NULL || s_framebuffer == NULL) {
        return;
    }

    /* Flush the local RGB565 framebuffer to the DMA-backed panel. */
    for (int y = 0; y < HUB75E_HEIGHT; y++) {
        const uint16_t *row = &s_framebuffer[y * HUB75E_WIDTH];
        for (int x = 0; x < HUB75E_WIDTH; x++) {
            s_dma_display->drawPixel(x, y, row[x]);
        }
    }

    /* On ESP32-S3 the CPU cache must be flushed so the DMA engine sees the updates. */
    s_dma_display->flushFrameBuffer();
}

void hub75e_driver_set_brightness(uint8_t brightness)
{
    if (brightness > 100) {
        brightness = 100;
    }
    s_brightness = brightness;
    if (s_dma_display != NULL) {
        /* Map 0-100 to 0-255 for the DMA library. */
        s_dma_display->setBrightness8((uint8_t)((brightness * 255) / 100));
    }
}

void hub75e_driver_show_test_pattern(uint32_t duration_ms, uint16_t color)
{
    if (!s_initialized || s_dma_display == NULL) {
        return;
    }

    ESP_LOGI(TAG, "Test pattern: color=0x%04X for %lu ms", color, duration_ms);

    /* Use drawRGBBitmap like the working reference project, in case fillScreen()
       behaves differently with the GFX backend on this panel. */
    size_t test_size = HUB75E_WIDTH * HUB75E_HEIGHT;
    uint16_t *test_bitmap = (uint16_t *)heap_caps_malloc(test_size * sizeof(uint16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (test_bitmap != NULL) {
        for (size_t i = 0; i < test_size; i++) {
            test_bitmap[i] = color;
        }
        s_dma_display->drawRGBBitmap(0, 0, test_bitmap, HUB75E_WIDTH, HUB75E_HEIGHT);
        s_dma_display->flushFrameBuffer();
        heap_caps_free(test_bitmap);
    } else {
        s_dma_display->fillScreen(color);
    }

    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    s_dma_display->clearScreen();

    /* Restore current framebuffer content. */
    hub75e_driver_refresh();
}
