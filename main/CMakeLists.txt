idf_component_register(SRCS "main.c" "mqtt/mqtt.c"
                    INCLUDE_DIRS "." 
                    REQUIRES esp_wifi nvs_flash esp_event esp_netif mqtt dht)
