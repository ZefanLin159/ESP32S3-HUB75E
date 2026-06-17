#include "ui_animations.h"
#include <string.h>
#include <math.h>

void ui_clear(uint16_t *fb, int width, int height, ui_color_t color)
{
    if (fb == NULL || width <= 0 || height <= 0) {
        return;
    }
    for (int i = 0; i < width * height; i++) {
        fb[i] = color;
    }
}

void ui_draw_pixel(uint16_t *fb, int width, int height, int x, int y, ui_color_t color)
{
    if (fb == NULL || x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    fb[y * width + x] = color;
}

void ui_draw_rect(uint16_t *fb, int width, int height,
                  int x, int y, int w, int h, ui_color_t color)
{
    if (fb == NULL || w <= 0 || h <= 0) {
        return;
    }
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            ui_draw_pixel(fb, width, height, xx, yy, color);
        }
    }
}

void ui_draw_spectrum_bars(uint16_t *fb, int width, int height,
                           const float *bins, int bin_count,
                           ui_color_t color)
{
    if (fb == NULL || bins == NULL || bin_count <= 0) {
        return;
    }

    int bar_width = width / bin_count;
    if (bar_width < 1) {
        bar_width = 1;
    }

    for (int i = 0; i < bin_count; i++) {
        float value = bins[i];
        if (value < 0.0f) {
            value = 0.0f;
        } else if (value > 1.0f) {
            value = 1.0f;
        }

        int bar_height = (int)(value * height);
        int x = i * bar_width;
        int y = height - bar_height;

        ui_draw_rect(fb, width, height, x, y, bar_width - 1, bar_height, color);
    }
}

ui_color_t ui_hsv_to_rgb565(float h, float s, float v)
{
    if (s <= 0.0f) {
        uint8_t c = (uint8_t)(v * 255.0f);
        return ui_rgb(c, c, c);
    }

    while (h < 0.0f) {
        h += 360.0f;
    }
    while (h >= 360.0f) {
        h -= 360.0f;
    }

    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r, g, b;
    int sector = (int)(h / 60.0f);
    switch (sector) {
    case 0:  r = c; g = x; b = 0.0f; break;
    case 1:  r = x; g = c; b = 0.0f; break;
    case 2:  r = 0.0f; g = c; b = x; break;
    case 3:  r = 0.0f; g = x; b = c; break;
    case 4:  r = x; g = 0.0f; b = c; break;
    default: r = c; g = 0.0f; b = x; break;
    }

    uint8_t rb = (uint8_t)((r + m) * 255.0f);
    uint8_t gb = (uint8_t)((g + m) * 255.0f);
    uint8_t bb = (uint8_t)((b + m) * 255.0f);
    return ui_rgb(rb, gb, bb);
}

void ui_draw_rainbow_breathing(uint16_t *fb, int width, int height, uint32_t tick_ms)
{
    if (fb == NULL || width <= 0 || height <= 0) {
        return;
    }

    /* One full hue cycle across the panel, drifting over ~4 seconds. */
    const float hue_drift_per_ms = 360.0f / 4000.0f;

    float hue_offset = fmodf(tick_ms * hue_drift_per_ms, 360.0f);

    /* Precompute one rainbow row; all rows are identical. */
    for (int x = 0; x < width; x++) {
        float hue = hue_offset + ((float)x / (float)width) * 360.0f;
        fb[x] = ui_hsv_to_rgb565(hue, 1.0f, 1.0f);
    }

    for (int y = 1; y < height; y++) {
        memcpy(&fb[y * width], fb, width * sizeof(uint16_t));
    }
}

void ui_draw_rainbow_carousel(uint16_t *fb, int width, int height, uint32_t tick_ms)
{
    if (fb == NULL || width <= 0 || height <= 0) {
        return;
    }

    /* 7 rainbow colors, each shown for about 0.714s to complete a cycle in ~5s. */
    const uint32_t color_duration_ms = 5000 / 7;
    const float rainbow_hues[7] = {0.0f, 30.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f};

    int idx = (tick_ms / color_duration_ms) % 7;
    ui_color_t color = ui_hsv_to_rgb565(rainbow_hues[idx], 1.0f, 1.0f);

    for (int i = 0; i < width * height; i++) {
        fb[i] = color;
    }
}

void ui_draw_rainbow_shrink_center(uint16_t *fb, int width, int height, uint32_t tick_ms)
{
    if (fb == NULL || width <= 0 || height <= 0) {
        return;
    }

    /* One full hue cycle over ~3 seconds, combined with distance from center. */
    const float hue_cycle_ms = 3000.0f;
    float hue_time_offset = fmodf(tick_ms * (360.0f / hue_cycle_ms), 360.0f);

    float center_x = (float)(width - 1) / 2.0f;
    float center_y = (float)(height - 1) / 2.0f;
    float max_dist = sqrtf(center_x * center_x + center_y * center_y);

    for (int y = 0; y < height; y++) {
        uint16_t *row = &fb[y * width];
        float dy = (float)y - center_y;
        for (int x = 0; x < width; x++) {
            float dx = (float)x - center_x;
            float dist = sqrtf(dx * dx + dy * dy);
            float normalized = dist / max_dist; /* 0.0 at center, 1.0 at corners. */

            float hue = hue_time_offset + normalized * 360.0f;
            row[x] = ui_hsv_to_rgb565(hue, 1.0f, 1.0f);
        }
    }
}

void ui_draw_boot_animation(uint16_t *fb, int width, int height,
                            boot_animation_t anim, uint32_t tick_ms)
{
    switch (anim) {
    case BOOT_ANIM_RAINBOW_CAROUSEL:
        ui_draw_rainbow_carousel(fb, width, height, tick_ms);
        break;
    case BOOT_ANIM_RAINBOW_BREATHING:
        ui_draw_rainbow_breathing(fb, width, height, tick_ms);
        break;
    case BOOT_ANIM_RAINBOW_SHRINK_CENTER:
        ui_draw_rainbow_shrink_center(fb, width, height, tick_ms);
        break;
    default:
        ui_draw_rainbow_breathing(fb, width, height, tick_ms);
        break;
    }
}
