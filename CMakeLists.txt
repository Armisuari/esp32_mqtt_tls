# ESP32-S3 MQTT TLS Example

cmake_minimum_required(VERSION 3.16)

# Set ESP-IDF target
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_mqtt_tls)