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
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

static const char *TAG = "MQTT_TLS_EXAMPLE";

// WiFi Configuration
#define WIFI_SSID "Noovoleum_Office"
#define WIFI_PASS "greenenergychampion"

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
static bool wifi_connected = false;
static int message_count = 0;

/*
 * Root CA certificate for test.mosquitto.org
 * Updated ISRG Root X1 certificate (Let's Encrypt)
 */
static const char *mqtt_server_cert_pem = "-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n";

/**
 * @brief WiFi event handler
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi started, connecting...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi disconnected, retrying...");
        wifi_connected = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
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
    
    ESP_LOGI(TAG, "WiFi initialization completed");
}

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
    ESP_LOGI(TAG, "Initializing MQTT client...");
    
    // Generate unique client ID based on MAC address
    uint8_t mac[6];
    char client_id[32];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(client_id, sizeof(client_id), "%s%02X%02X%02X", MQTT_CLIENT_ID, mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "Generated client ID: %s", client_id);
    
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

    ESP_LOGI(TAG, "Calling esp_mqtt_client_init()...");
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }
    ESP_LOGI(TAG, "MQTT client initialized successfully");
    
    /* Register event handler */
    ESP_LOGI(TAG, "Registering MQTT event handler...");
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    ESP_LOGI(TAG, "Event handler registered successfully");
    
    /* Start MQTT client */
    ESP_LOGI(TAG, "Starting MQTT client...");
    esp_err_t err = esp_mqtt_client_start(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "MQTT client start command sent successfully");
    
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
    ESP_LOGI(TAG, "Initializing WiFi...");
    wifi_init();
    
    // Wait for WiFi connection
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    int retry_count = 0;
    while (!wifi_connected && retry_count < 30) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry_count++;
        ESP_LOGI(TAG, "Waiting for WiFi... (%d/30)", retry_count);
    }
    
    if (!wifi_connected) {
        ESP_LOGE(TAG, "Failed to connect to WiFi after 30 seconds");
        return;
    }
    
    ESP_LOGI(TAG, "WiFi connected successfully!");
    
    // Add delay to ensure network is fully ready
    ESP_LOGI(TAG, "Waiting 2 seconds for network to stabilize...");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "Starting MQTT client...");
    
    // Start MQTT client
    mqtt_app_start();
    
    // Create sensor data publishing task
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Application started. Publishing to: %s", MQTT_TOPIC_PUBLISH);
    ESP_LOGI(TAG, "Subscribed to commands on: %s", MQTT_TOPIC_SUBSCRIBE);
    ESP_LOGI(TAG, "Send commands to see echo responses!");
}