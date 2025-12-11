/*
 * ESP32-S3 MQTT TLS Client Example
 * 
 * This example demonstrates secure MQTT over TLS/SSL connection
 * using ESP32-S3 with ESP-IDF v5.3.x
 */

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MQTT_TLS_CLIENT";

// WiFi Configuration - Update these with your network
#define WIFI_SSID "Noovoleum_Office"
#define WIFI_PASS "greenenergychampion"

// MQTT Configuration - Update with your broker IP
#define MQTT_BROKER_URL "mqtts://192.168.19.50:8883"
#define MQTT_TOPIC "test/esp32"

// Global variables
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool wifi_connected = false;
static bool mqtt_connected = false;

/*
 * CA Certificate for TLS verification
 */
static const char *test_cert_pem =
"-----BEGIN CERTIFICATE-----\n"
"MIIDkTCCAnmgAwIBAgIUKw3sgVcES4OLkbkhyG206ANOswgwDQYJKoZIhvcNAQEL\n"
"BQAwWDELMAkGA1UEBhMCVVMxDjAMBgNVBAgMBVN0YXRlMQ0wCwYDVQQHDARDaXR5\n"
"MRQwEgYDVQQKDAtNb3NxdWl0dG9DQTEUMBIGA1UEAwwLTW9zcXVpdHRvQ0EwHhcN\n"
"MjUxMjExMDcxNzU2WhcNMzUxMjA5MDcxNzU2WjBYMQswCQYDVQQGEwJVUzEOMAwG\n"
"A1UECAwFU3RhdGUxDTALBgNVBAcMBENpdHkxFDASBgNVBAoMC01vc3F1aXR0b0NB\n"
"MRQwEgYDVQQDDAtNb3NxdWl0dG9DQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\n"
"AQoCggEBAM97R4exgK025wnkkBJu1NIBuC5UypVSwN3rRBHv8NOZtDZ+QEhHqZUm\n"
"j1M6F54Wzt0FVVnwmYXinJf7odd9GYfNkiHjEbu/IW1Ct4b4DGUHLC5QRC9au866\n"
"JGTrCTQop63UTEee6eCt+3L3UuuBIokyTjwkZSrMLAORJjVl353Mm+xdoaazcYOi\n"
"bbWONlPdkNKb+uc3x1xD058BFa13/jfN4djpC6aebniyyxYTTB3GbZ+296Lq6+Hc\n"
"ZPqvOKpTtsWqrkPrmBIQ7eQTl4BiDe+Hagm7F7f4/Jme+LBDOIAhoxvfd5uSPNdJ\n"
"j1HkVB+xUK1+CYCeNc/OcQhcYUsUqjMCAwEAAaNTMFEwHQYDVR0OBBYEFIDaIqb1\n"
"+ryYNG4dd8M9dhNrRKVaMB8GA1UdIwQYMBaAFIDaIqb1+ryYNG4dd8M9dhNrRKVa\n"
"MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBALAAZaP5Tz10LRLI\n"
"sPSC2BzV0RxtNDICWc6pYmzXO2mcEI6in1Ylt4mE7rM0IEDP9QJP3Se/DGiD/1LZ\n"
"tKgWTPxwmwVHrA5k+mm50INt0+IiABKsPfh42EqOQcjxZ8hh8lSU8j+jubSLXECe\n"
"QDJwlJBUULLAGwcSz2V6M1V4X/i4uSyzF3fPJ0tX2tFNuZHoCPQntwiAi8GtVA7O\n"
"pO81CUDpJb2E4t9vXbZt/YUG6QFb7FjHhVoNuH6yIYiPL+thbI/sEdWEP9ltECeh\n"
"DopbSsXEQkF3iVfOW5Hd/+5HNLcNhfPRomVQy/bq7HdMpE5w16l8ZPBOjXcR1djV\n"
"1Xrw9q0=\n"
"-----END CERTIFICATE-----\n";

/**
 * @brief WiFi event handler
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WiFi started, connecting...");
            esp_wifi_connect();
            break;
            
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "WiFi disconnected, retrying...");
            wifi_connected = false;
            esp_wifi_connect();
            break;
            
        case IP_EVENT_STA_GOT_IP:
            {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
                wifi_connected = true;
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief Initialize WiFi
 */
static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @brief MQTT event handler
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "🔒 MQTT TLS Connected!");
            mqtt_connected = true;
            
            // Subscribe to a test topic
            esp_mqtt_client_subscribe(event->client, MQTT_TOPIC, 1);
            ESP_LOGI(TAG, "Subscribed to topic: %s", MQTT_TOPIC);
            
            // Send a test message
            esp_mqtt_client_publish(event->client, MQTT_TOPIC, "Hello from ESP32-S3 TLS!", 0, 1, 0);
            ESP_LOGI(TAG, "Published test message");
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            mqtt_connected = false;
            break;
            
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT Subscribed, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "📨 Received message:");
            ESP_LOGI(TAG, "Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Data: %.*s", event->data_len, event->data);
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT Error occurred");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "TLS error: 0x%x", event->error_handle->esp_tls_last_esp_err);
            }
            break;
            
        default:
            ESP_LOGD(TAG, "MQTT event: %ld", event_id);
            break;
    }
}

/**
 * @brief Initialize and start MQTT client with TLS
 */
static void mqtt_init(void)
{
    ESP_LOGI(TAG, "🔐 Initializing MQTT TLS client...");
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URL,
            .verification.certificate = test_cert_pem,  // TLS certificate
        },
        .credentials = {
            .client_id = "esp32_tls_basic",
            // No username/password for this basic example
        }
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }
    
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
    
    ESP_LOGI(TAG, "MQTT TLS client started");
}

/**
 * @brief Task to send periodic messages
 */
static void mqtt_publish_task(void *pvParameters)
{
    int counter = 0;
    char message[64];
    
    while (1) {
        if (mqtt_connected) {
            snprintf(message, sizeof(message), "Message #%d from ESP32-S3 TLS", ++counter);
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC, message, 0, 1, 0);
            ESP_LOGI(TAG, "📤 Published: %s", message);
        } else {
            ESP_LOGW(TAG, "MQTT not connected, waiting...");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); // Send every 10 seconds
    }
}

/**
 * @brief Main application
 */
void app_main(void)
{
    ESP_LOGI(TAG, "🚀 ESP32-S3 MQTT TLS Client Starting...");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize WiFi
    wifi_init();
    
    // Wait for WiFi connection
    ESP_LOGI(TAG, "📶 Waiting for WiFi connection...");
    while (!wifi_connected) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG, "✅ WiFi connected!");
    
    // Give network a moment to stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Initialize MQTT with TLS
    mqtt_init();
    
    // Wait for MQTT connection
    ESP_LOGI(TAG, "🔗 Waiting for MQTT TLS connection...");
    while (!mqtt_connected) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG, "✅ MQTT TLS connected!");
    
    // Start publishing task
    xTaskCreate(mqtt_publish_task, "mqtt_pub_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "🎉 MQTT TLS Client is running!");
    ESP_LOGI(TAG, "Connecting to: %s", MQTT_BROKER_URL);
    ESP_LOGI(TAG, "Topic: %s", MQTT_TOPIC);
}