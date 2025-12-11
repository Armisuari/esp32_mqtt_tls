idf_component_register(SRCS "mqtt_tls_client.c"
                    INCLUDE_DIRS "."
                    REQUIRES mqtt protocol_examples_common esp_partition nvs_flash esp_wifi)