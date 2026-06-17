#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MSG_NONE = 0,

    /* Button events (param: raw key id, optional) */
    MSG_BUTTON_LEFT_SHORT,
    MSG_BUTTON_LEFT_LONG,
    MSG_BUTTON_RIGHT_SHORT,
    MSG_BUTTON_RIGHT_LONG,
    MSG_BUTTON_OK_SHORT,
    MSG_BUTTON_OK_LONG,
    MSG_BUTTON_COMBO_MENU,      /* OK + LEFT */
    MSG_BUTTON_COMBO_SETTINGS,  /* OK + RIGHT */

    /* Audio control */
    MSG_PLAY_NEXT,
    MSG_PLAY_PREV,
    MSG_PLAY_PAUSE,
    MSG_VOLUME_UP,
    MSG_VOLUME_DOWN,
    MSG_AUDIO_TRACK_STARTED,
    MSG_AUDIO_TRACK_FINISHED,

    /* Mode / system */
    MSG_MODE_SWITCH,
    MSG_SYSTEM_SLEEP,
    MSG_SYSTEM_WAKE,

    /* Data-ready notifications */
    MSG_NEW_SPECTRUM_DATA,
    MSG_DISPLAY_UPDATE,

    /* Storage events */
    MSG_STORAGE_MOUNTED,
    MSG_STORAGE_UNMOUNTED,
    MSG_STORAGE_SCAN_DONE,
} message_type_t;

typedef struct {
    message_type_t type;
    int32_t param;      /* small scalar payload */
    void *data;         /* pointer to shared/static data, NULL if none */
    size_t data_len;    /* length of data in bytes */
} message_t;

/**
 * @brief Initialize the inter-agent message broker.
 *
 * Must be called before any task starts sending messages.
 */
esp_err_t message_broker_init(void);

/**
 * @brief Send a message to the broker queue.
 *
 * The caller retains ownership of any pointed-to data; use shared/locked
 * buffers only. For simple events, set data to NULL and data_len to 0.
 */
esp_err_t send_message(message_type_t type, int32_t param, void *data, size_t data_len);

/**
 * @brief Receive a message from the broker queue.
 *
 * Blocks up to @p timeout ticks. Returns ESP_ERR_TIMEOUT if no message.
 */
esp_err_t receive_message(message_t *msg, TickType_t timeout);

#ifdef __cplusplus
}
#endif
