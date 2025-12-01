/*
 * ESP32-S3 MQTT TLS Example
 * 
 * This example demonstrates MQTT over TLS/SSL connection to test.mosquitto.org
 * using ESP32-S3 with ESP-IDF v5.3.4
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_random.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

static const char *TAG = "MQTT_TLS_EXAMPLE";

// MQTT Configuration
#define MQTT_BROKER_URL "mqtts://test.mosquitto.org:8883"
#define MQTT_CLIENT_ID "ESP32S3_" 
#define MQTT_USERNAME NULL
#define MQTT_PASSWORD NULL

// Topics
#define MQTT_TOPIC_PUBLISH "deme25/8/esp32s3/sensor/temperature"
#define MQTT_TOPIC_SUBSCRIBE "deme25/8/esp32s3/commands"

// Global variables
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;
static int message_count = 0;

/*
 * Root CA certificate for test.mosquitto.org
 * This is the DST Root CA X3 certificate that signs the mosquitto server cert
 */
static const char *mqtt_server_cert_pem = "-----BEGIN CERTIFICATE-----\n"
"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n"
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n"
"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n"
"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n"
"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n"
"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n"
"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n"
"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n"
"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n"
"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n"
"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n"
"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n"
"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n"
"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n"
"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n"
"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n"
"-----END CERTIFICATE-----\n";

/**
 * @brief MQTT event handler
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_connected = true;
        
        // Subscribe to command topic
        int msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUBSCRIBE, 1);
        ESP_LOGI(TAG, "Subscribed to %s, msg_id=%d", MQTT_TOPIC_SUBSCRIBE, msg_id);
        
        // Publish a connect message
        msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_PUBLISH, "ESP32-S3 Connected!", 0, 1, 0);
        ESP_LOGI(TAG, "Published connect message, msg_id=%d", msg_id);
        break;
        
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_connected = false;
        break;
        
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG, "Topic: %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "Data: %.*s", event->data_len, event->data);
        
        // Echo received commands
        char response[128];
        snprintf(response, sizeof(response), "Received: %.*s", event->data_len, event->data);
        esp_mqtt_client_publish(client, MQTT_TOPIC_PUBLISH, response, 0, 1, 0);
        break;
        
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
        
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

/**
 * @brief Initialize and start MQTT client
 */
static void mqtt_app_start(void)
{
    // Generate unique client ID based on MAC address
    uint8_t mac[6];
    char client_id[32];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(client_id, sizeof(client_id), "%s%02X%02X%02X", MQTT_CLIENT_ID, mac[3], mac[4], mac[5]);
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URL,
            .verification.certificate = mqtt_server_cert_pem,
        },
        .credentials = {
            .client_id = client_id,
            .username = MQTT_USERNAME,
            .authentication.password = MQTT_PASSWORD,
        }
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }
    
    /* Register event handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    
    /* Start MQTT client */
    esp_err_t err = esp_mqtt_client_start(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        return;
    }
    
    ESP_LOGI(TAG, "MQTT client started with ID: %s", client_id);
}

/**
 * @brief Task to publish sensor data periodically
 */
static void sensor_task(void *pvParameters)
{
    char payload[128];
    float temperature = 25.0; // Simulated temperature
    
    while (1) {
        if (mqtt_connected) {
            // Simulate temperature reading with some variation
            temperature += ((float)(esp_random() % 200) - 100) / 100.0; // +/- 1.0°C variation
            if (temperature < 20.0) temperature = 20.0;
            if (temperature > 30.0) temperature = 30.0;
            
            // Create JSON payload
            snprintf(payload, sizeof(payload), 
                    "{\"device\":\"ESP32-S3\",\"sensor\":\"temperature\",\"value\":%.2f,\"unit\":\"C\",\"count\":%d}", 
                    temperature, ++message_count);
            
            // Publish sensor data
            int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_PUBLISH, payload, 0, 1, 0);
            ESP_LOGI(TAG, "Published sensor data: %s (msg_id=%d)", payload, msg_id);
        } else {
            ESP_LOGW(TAG, "MQTT not connected, skipping sensor publish");
        }
        
        // Wait 10 seconds before next reading
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief Main application entry point
 */
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 MQTT TLS Example Starting...");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    // Print chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "Chip: %s with %d CPU core(s), WiFi%s%s",
             (chip_info.model == CHIP_ESP32S3) ? "ESP32-S3" : "Unknown",
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize network stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    /* Connect to WiFi network */
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "WiFi connected successfully");
    
    // Start MQTT client
    mqtt_app_start();
    
    // Create sensor data publishing task
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Application started. Publishing to: %s", MQTT_TOPIC_PUBLISH);
    ESP_LOGI(TAG, "Subscribed to commands on: %s", MQTT_TOPIC_SUBSCRIBE);
    ESP_LOGI(TAG, "Send commands to see echo responses!");
}