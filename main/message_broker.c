#include "message_broker.h"
#include "esp_log.h"

#define MESSAGE_QUEUE_LENGTH 32

static const char *TAG = "msg_broker";
static QueueHandle_t s_message_queue = NULL;

esp_err_t message_broker_init(void)
{
    if (s_message_queue != NULL) {
        return ESP_OK;
    }

    s_message_queue = xQueueCreate(MESSAGE_QUEUE_LENGTH, sizeof(message_t));
    if (s_message_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create message queue");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Message broker initialized");
    return ESP_OK;
}

esp_err_t send_message(message_type_t type, int32_t param, void *data, size_t data_len)
{
    if (s_message_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    message_t msg = {
        .type = type,
        .param = param,
        .data = data,
        .data_len = data_len,
    };

    if (xQueueSend(s_message_queue, &msg, pdMS_TO_TICKS(10)) != pdPASS) {
        ESP_LOGW(TAG, "Message queue full, dropped type=%d", (int)type);
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

esp_err_t receive_message(message_t *msg, TickType_t timeout)
{
    if (s_message_queue == NULL || msg == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xQueueReceive(s_message_queue, msg, timeout) != pdPASS) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}
