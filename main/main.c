#include "wifi/wifi.h"
#include "mqtt/mqtt.h"

xSemaphoreHandle wifiConnection;
xSemaphoreHandle mqttConnection;

void wifi_treat(void *args)
{
    while(true)
    {
        if(xSemaphoreTake(wifiConnection, portMAX_DELAY))
        {
            mqtt_start();
        }
    }
}

void mqtt_treat(void *args)
{
    char msg[50];
    float fake_temperatura;
    if(xSemaphoreTake(mqttConnection, portMAX_DELAY))
    {
        while(true)
        {
            fake_temperatura = 20.0 + (float)rand()/(float)(RAND_MAX/10.0);
            sprintf(msg, "{\n  \"data\":\n  {\n    \"Test\": %f\n  }\n}", 
                                                            fake_temperatura);
            mqtt_publish_msg("wnology/66cb41ac09d56a857e5fdda2/state", msg);
            printf("{\n  \"data\":\n  {\n    \"Test\": %f\n  }\n}", 
                                                            fake_temperatura);
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == 
                                                ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    wifiConnection = xSemaphoreCreateBinary();
    mqttConnection = xSemaphoreCreateBinary();

    wifi_connect();

    xTaskCreate(wifi_treat, "Tratamento Wifi", 4096, NULL, 1, NULL);
    xTaskCreate(mqtt_treat, "Tratamento Mqtt", 4096, NULL, 1, NULL);
}
