#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"


#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt.h"

static const char *TAG = "MQTT";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) 
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client; 
    int msg_id;

    switch (event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "wnology/66cb41ac09d56a857e5fdda2/state", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == 
                                                MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", 
                                    event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", 
                                    event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  
                                event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", 
                    strerror(event->error_handle->esp_transport_sock_errno));

            }
            vTaskDelay(pdMS_TO_TICKS(300));
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            vTaskDelay(pdMS_TO_TICKS(300));
            break;
    }
    return ESP_OK;                  
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data)
{
    ESP_LOGW(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", event_base, event_id);
    mqtt_event_handler_cb(event_data); 

    vTaskDelay(pdMS_TO_TICKS(300));                       
}

void mqtt_publish_task(void *args) {
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t) args; 
    int msg_id;
    //char json_data[100];
    //snprintf(json_data, sizeof(json_data), "{\n\"data\": {\n\"aStringAttribute\":\"Test\",\n\"aNumberAttribute\":42\n},}");
    while (true) {
        
        msg_id = esp_mqtt_client_publish(client, "wnology/66cb41ac09d56a857e5fdda2/command", "123", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        vTaskDelay(pdMS_TO_TICKS(3000));  // Delay de 3 segundos
    }
}

void mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_config = 
    {
        .broker.address.uri = "",
        .broker.address.port = 1883,
        .credentials.username = "",
        .credentials.authentication.password = 
            "",
        .credentials.client_id = "",
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, 
                                                    mqtt_event_handler, client);
    esp_mqtt_client_start(client);    

    xTaskCreate(mqtt_publish_task, "mqtt_publish_task", 4096, (void*)client, 5, NULL);                                                
}