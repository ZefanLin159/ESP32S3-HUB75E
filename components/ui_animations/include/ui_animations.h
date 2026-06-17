#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t ui_color_t;

static inline ui_color_t ui_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (ui_color_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

void ui_clear(uint16_t *fb, int width, int height, ui_color_t color);

void ui_draw_pixel(uint16_t *fb, int width, int height, int x, int y, ui_color_t color);

void ui_draw_rect(uint16_t *fb, int width, int height,
                  int x, int y, int w, int h, ui_color_t color);

void ui_draw_spectrum_bars(uint16_t *fb, int width, int height,
                           const float *bins, int bin_count,
                           ui_color_t color);

/**
 * @brief Convert HSV to RGB565.
 *
 * @param h Hue in degrees [0, 360).
 * @param s Saturation [0, 1].
 * @param v Value/Brightness [0, 1].
 * @return RGB565 color.
 */
ui_color_t ui_hsv_to_rgb565(float h, float s, float v);

/**
 * @brief Draw a horizontal rainbow breathing gradient.
 *
 * Hue shifts horizontally across the panel and drifts over time.
 * Overall brightness pulses (breathes) with a sine wave.
 *
 * @param fb       RGB565 framebuffer.
 * @param width    Panel width.
 * @param height   Panel height.
 * @param tick_ms  Elapsed time in milliseconds.
 */
void ui_draw_rainbow_breathing(uint16_t *fb, int width, int height, uint32_t tick_ms);

/**
 * @brief Draw a full-screen solid rainbow carousel.
 *
 * Cycles through the seven rainbow colors (red/orange/yellow/green/cyan/blue/purple).
 *
 * @param fb       RGB565 framebuffer.
 * @param width    Panel width.
 * @param height   Panel height.
 * @param tick_ms  Elapsed time in milliseconds.
 */
void ui_draw_rainbow_carousel(uint16_t *fb, int width, int height, uint32_t tick_ms);

/**
 * @brief Draw a seven-color radial gradient shrinking toward the center.
 *
 * Hue is driven by pixel distance from the screen center plus elapsed time,
 * creating a contracting/expanding rainbow effect.
 *
 * @param fb       RGB565 framebuffer.
 * @param width    Panel width.
 * @param height   Panel height.
 * @param tick_ms  Elapsed time in milliseconds.
 */
void ui_draw_rainbow_shrink_center(uint16_t *fb, int width, int height, uint32_t tick_ms);

/**
 * @brief Boot animation modes.
 */
typedef enum {
    BOOT_ANIM_RAINBOW_CAROUSEL = 0,
    BOOT_ANIM_RAINBOW_BREATHING,
    BOOT_ANIM_RAINBOW_SHRINK_CENTER,
    BOOT_ANIM_COUNT
} boot_animation_t;

/**
 * @brief Dispatcher for boot animations.
 *
 * @param fb       RGB565 framebuffer.
 * @param width    Panel width.
 * @param height   Panel height.
 * @param anim     Animation mode.
 * @param tick_ms  Elapsed time in milliseconds.
 */
void ui_draw_boot_animation(uint16_t *fb, int width, int height,
                            boot_animation_t anim, uint32_t tick_ms);

#ifdef __cplusplus
}
#endif
