#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_MOUNT_POINT "/sdcard"
#define STORAGE_MUSIC_DIR   STORAGE_MOUNT_POINT "/MUSIC"
#define STORAGE_IMAGE_DIR   STORAGE_MOUNT_POINT "/IMAGE"

#define STORAGE_MAX_FILES 64
#define STORAGE_MAX_PATH  320

/**
 * @brief Initialize SD card and mount FAT filesystem.
 */
esp_err_t storage_agent_init(void);

/**
 * @brief Scan /MUSIC and /IMAGE directories and cache file lists.
 */
esp_err_t storage_agent_scan(void);

/**
 * @brief Return the number of cached music files.
 */
int storage_get_music_count(void);

/**
 * @brief Return the full path of the i-th music file.
 */
const char *storage_get_music_path(int index);

/**
 * @brief Return the number of cached image files.
 */
int storage_get_image_count(void);

/**
 * @brief Return the full path of the i-th image file.
 */
const char *storage_get_image_path(int index);

#ifdef __cplusplus
}
#endif
