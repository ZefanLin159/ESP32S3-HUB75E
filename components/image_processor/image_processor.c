#include "image_processor.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

#include "esp_log.h"

static const char *TAG = "image_processor";

/* BMP file header structures. */
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} bmp_file_header_t;

typedef struct {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_ppm;
    int32_t y_ppm;
    uint32_t colors_used;
    uint32_t colors_important;
} bmp_info_header_t;
#pragma pack(pop)

static inline uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static const char *get_extension(const char *path)
{
    const char *dot = strrchr(path, '.');
    if (dot == NULL) {
        return "";
    }
    return dot;
}

/* -------------------------------------------------------------------------- */
/* Scaling helpers                                                            */
/* -------------------------------------------------------------------------- */

static void compute_fit(int src_w, int src_h, int dst_w, int dst_h,
                        image_fit_t fit,
                        float *scale, float *src_x0, float *src_y0)
{
    float scale_x = (float)dst_w / (float)src_w;
    float scale_y = (float)dst_h / (float)src_h;

    if (fit == IMG_FIT_STRETCH) {
        *scale = 1.0f; /* unused for stretch */
        *src_x0 = 0.0f;
        *src_y0 = 0.0f;
        return;
    }

    if (fit == IMG_FIT_COVER) {
        *scale = fmaxf(scale_x, scale_y);
    } else { /* IMG_FIT_CONTAIN */
        *scale = fminf(scale_x, scale_y);
    }

    float scaled_w = src_w * (*scale);
    float scaled_h = src_h * (*scale);
    *src_x0 = ((float)dst_w - scaled_w) / (2.0f * (*scale));
    *src_y0 = ((float)dst_h - scaled_h) / (2.0f * (*scale));
}

static inline int clamp_int(int v, int min, int max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static void scale_row_nearest(const uint8_t *src_row, int src_w,
                              uint16_t *dst_row, int dst_w,
                              int bpp, float src_x0, float scale)
{
    for (int dx = 0; dx < dst_w; dx++) {
        float sx;
        if (scale <= 0.0f) {
            /* Stretch mode: direct mapping. */
            sx = (float)dx * (float)src_w / (float)dst_w;
        } else {
            sx = src_x0 + (float)dx / scale;
        }
        int ix = clamp_int((int)sx, 0, src_w - 1);
        const uint8_t *p = &src_row[ix * bpp];
        uint8_t b = p[0];
        uint8_t g = p[1];
        uint8_t r = p[2];
        dst_row[dx] = rgb888_to_rgb565(r, g, b);
    }
}

static void fill_black(uint16_t *fb, int count)
{
    for (int i = 0; i < count; i++) {
        fb[i] = 0x0000;
    }
}

/* -------------------------------------------------------------------------- */
/* BMP loader with streaming scaling                                          */
/* -------------------------------------------------------------------------- */

static esp_err_t load_bmp_streaming(const char *path, uint16_t *out_fb,
                                    int dst_w, int dst_h, image_fit_t fit)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open %s", path);
        return ESP_ERR_NOT_FOUND;
    }

    bmp_file_header_t file_hdr;
    bmp_info_header_t info_hdr;

    if (fread(&file_hdr, sizeof(file_hdr), 1, f) != 1 ||
        fread(&info_hdr, sizeof(info_hdr), 1, f) != 1) {
        ESP_LOGE(TAG, "Failed to read BMP headers from %s", path);
        fclose(f);
        return ESP_ERR_INVALID_SIZE;
    }

    if (file_hdr.type != 0x4D42) { /* 'BM' */
        ESP_LOGE(TAG, "Not a valid BMP file: %s", path);
        fclose(f);
        return ESP_ERR_NOT_SUPPORTED;
    }

    int src_w = (int)info_hdr.width;
    int src_h = (int)info_hdr.height;
    int bpp = info_hdr.bpp;
    int row_padding;
    bool top_down = false;

    if (src_h < 0) {
        src_h = -src_h;
        top_down = true;
    }

    if (bpp != 24 && bpp != 32) {
        ESP_LOGE(TAG, "Unsupported BMP bpp %d in %s", bpp, path);
        fclose(f);
        return ESP_ERR_NOT_SUPPORTED;
    }

    if (info_hdr.compression != 0) {
        ESP_LOGE(TAG, "Compressed BMP not supported: %s", path);
        fclose(f);
        return ESP_ERR_NOT_SUPPORTED;
    }

    int bytes_per_pixel = bpp / 8;
    row_padding = ((src_w * bytes_per_pixel + 3) & ~3) - (src_w * bytes_per_pixel);

    /* Row buffer for one source line in BGR(A) format. */
    uint8_t *src_row = (uint8_t *)malloc(src_w * bytes_per_pixel);
    if (src_row == NULL) {
        fclose(f);
        return ESP_ERR_NO_MEM;
    }

    fill_black(out_fb, dst_w * dst_h);

    float scale, src_x0, src_y0;
    compute_fit(src_w, src_h, dst_w, dst_h, fit, &scale, &src_x0, &src_y0);

    for (int dy = 0; dy < dst_h; dy++) {
        float sy;
        if (fit == IMG_FIT_STRETCH) {
            sy = (float)dy * (float)src_h / (float)dst_h;
        } else {
            sy = src_y0 + (float)dy / scale;
        }
        int iy = clamp_int((int)sy, 0, src_h - 1);

        /* BMP rows are stored bottom-up by default. */
        int file_row = top_down ? iy : (src_h - 1 - iy);
        long row_offset = (long)file_hdr.offset +
                          (long)file_row * (src_w * bytes_per_pixel + row_padding);
        if (fseek(f, row_offset, SEEK_SET) != 0) {
            ESP_LOGE(TAG, "Failed to seek BMP row %d", file_row);
            free(src_row);
            fclose(f);
            return ESP_FAIL;
        }
        if (fread(src_row, bytes_per_pixel, src_w, f) != (size_t)src_w) {
            ESP_LOGE(TAG, "Failed to read BMP row %d", file_row);
            free(src_row);
            fclose(f);
            return ESP_FAIL;
        }

        scale_row_nearest(src_row, src_w, &out_fb[dy * dst_w], dst_w,
                          bytes_per_pixel, src_x0, fit == IMG_FIT_STRETCH ? 0.0f : scale);
    }

    free(src_row);
    fclose(f);
    return ESP_OK;
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

esp_err_t image_load_to_rgb565(const char *path,
                                uint16_t *out_fb,
                                int out_w, int out_h,
                                image_fit_t fit)
{
    if (path == NULL || out_fb == NULL || out_w <= 0 || out_h <= 0) {
        return ESP_ERR_INVALID_ARG;
    }

    const char *ext = get_extension(path);
    if (strcasecmp(ext, ".bmp") == 0) {
        return load_bmp_streaming(path, out_fb, out_w, out_h, fit);
    }

    ESP_LOGW(TAG, "Unsupported image format: %s", path);
    return ESP_ERR_NOT_SUPPORTED;
}

bool image_is_animated(const char *path)
{
    if (path == NULL) {
        return false;
    }
    const char *ext = get_extension(path);
    return (strcasecmp(ext, ".gif") == 0);
}

esp_err_t image_load_frame_to_rgb565(const char *path,
                                      int frame_index,
                                      uint16_t *out_fb,
                                      int out_w, int out_h,
                                      image_fit_t fit)
{
    (void)frame_index;
    if (!image_is_animated(path)) {
        return image_load_to_rgb565(path, out_fb, out_w, out_h, fit);
    }
    /* GIF support is reserved for a later iteration. */
    ESP_LOGW(TAG, "GIF decoding is not yet implemented");
    return ESP_ERR_NOT_SUPPORTED;
}

int image_frame_count(const char *path)
{
    if (image_is_animated(path)) {
        /* GIF support reserved. */
        return 1;
    }
    return 1;
}

int image_frame_delay_ms(const char *path, int frame_index)
{
    (void)path;
    (void)frame_index;
    return 100; /* Default 100 ms for static/placeholder. */
}
