#include "storage_agent.h"
#include "message_broker.h"
#include "sdkconfig.h"

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"

static const char *TAG = "storage";

static sdmmc_card_t *s_card = NULL;

static char s_music_files[STORAGE_MAX_FILES][STORAGE_MAX_PATH];
static int s_music_count = 0;

static char s_image_files[STORAGE_MAX_FILES][STORAGE_MAX_PATH];
static int s_image_count = 0;

static int scan_directory(const char *dir_path, char files[][STORAGE_MAX_PATH], int max_files)
{
    DIR *dir = opendir(dir_path);
    if (!dir) {
        ESP_LOGW(TAG, "Failed to open directory %s", dir_path);
        return 0;
    }

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < max_files) {
        if (entry->d_type != DT_REG) {
            continue;
        }
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len < 4) {
            continue;
        }
        const char *ext = name + len - 4;
        if (strcasecmp(ext, ".mp3") != 0 &&
            strcasecmp(ext, ".wav") != 0 &&
            strcasecmp(ext, ".bmp") != 0 &&
            strcasecmp(ext, ".jpg") != 0 &&
            strcasecmp(ext, ".png") != 0) {
            continue;
        }

        snprintf(files[count], STORAGE_MAX_PATH, "%s/%s", dir_path, name);
        count++;
    }

    closedir(dir);
    return count;
}

esp_err_t storage_agent_init(void)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = CONFIG_PMC_SD_MOSI_GPIO,
        .miso_io_num = CONFIG_PMC_SD_MISO_GPIO,
        .sclk_io_num = CONFIG_PMC_SD_SCLK_GPIO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 4092,
    };

    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CONFIG_PMC_SD_CS_GPIO;
    slot_config.host_id = host.slot;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    ret = esp_vfs_fat_sdspi_mount(STORAGE_MOUNT_POINT, &host, &slot_config, &mount_config, &s_card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SD card mounted at %s", STORAGE_MOUNT_POINT);
    send_message(MSG_STORAGE_MOUNTED, 0, NULL, 0);
    return ESP_OK;
}

esp_err_t storage_agent_scan(void)
{
    s_music_count = scan_directory(STORAGE_MUSIC_DIR, s_music_files, STORAGE_MAX_FILES);
    s_image_count = scan_directory(STORAGE_IMAGE_DIR, s_image_files, STORAGE_MAX_FILES);

    ESP_LOGI(TAG, "Scanned %d music files, %d image files", s_music_count, s_image_count);
    send_message(MSG_STORAGE_SCAN_DONE, (s_music_count << 16) | s_image_count, NULL, 0);
    return ESP_OK;
}

int storage_get_music_count(void)
{
    return s_music_count;
}

const char *storage_get_music_path(int index)
{
    if (index < 0 || index >= s_music_count) {
        return NULL;
    }
    return s_music_files[index];
}

int storage_get_image_count(void)
{
    return s_image_count;
}

const char *storage_get_image_path(int index)
{
    if (index < 0 || index >= s_image_count) {
        return NULL;
    }
    return s_image_files[index];
}
