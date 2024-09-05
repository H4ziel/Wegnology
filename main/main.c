#include "wifi/wifi.h"
#include "mqtt/mqtt.h"
#include "dht11.h"

#define DHT_PIN 4

const char *TAG = "DHT11";

xSemaphoreHandle wifiConnection;
xSemaphoreHandle mqttConnection;

typedef struct{
    int temp;
    int humidity;
    char *msg;
}measures;

measures *variables;

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
    measures *vars = (measures*)args;
    if(xSemaphoreTake(mqttConnection, portMAX_DELAY))
    {
        while(true)
        {  
            vars->temp = DHT11_read().temperature;
            vars->humidity = DHT11_read().humidity;
            sprintf(vars->msg, "{\n  \"data\":\n  {\n    \"Temperatura\": %d,\n    \"Umidade\": %d\n  }\n}", 
                                                    vars->temp, vars->humidity);
            mqtt_publish_msg("wnology/66d2514337e3c689cf394eac/state", 
                                                                     vars->msg);
            printf(vars->msg);
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

    DHT11_init(DHT_PIN);

    xTaskCreatePinnedToCore(wifi_treat, "Tratamento Wifi", 3000, NULL, 1, NULL,
                                                                             0);
    xTaskCreatePinnedToCore(mqtt_treat, "Tratamento Mqtt", 3000, 
                                                 (void*)&variables, 1, NULL, 0);
}
