/*
 * config.h
 *
 *  Created on: 24.12.2021
 *
 * Copyright (C) 2021 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include "utils.h"

/**
 * This file contains a few variables and defines to be used as config values.
 */

// Deep sleep mode options
// Deep sleep mode makes the esp go into deep sleep for a fixed time before makeing a measurement, pushing it,
// and going into deep sleep again.
// Set to 1 to enable and to 0 to disable.
// The ifndef is there to allow setting deep sleep mode using the command line.
#ifndef ENABLE_DEEP_SLEEP_MODE
#define ENABLE_DEEP_SLEEP_MODE 0
#endif
#if ENABLE_DEEP_SLEEP_MODE == 1
// The time between two measurements in deep sleep mode.
// Specified in seconds.
// Default is 5 minutes, or 300 seconds.
static constexpr uint32_t DEEP_SLEEP_MODE_MEASUREMENT_INTERVAL = 30;

// Deep sleep mode automatic config.
#undef ENABLE_WEB_SERVER
#define ENABLE_WEB_SERVER 0
#undef ENABLE_ARDUINO_OTA
#define ENABLE_ARDUINO_OTA 0
#undef ENABLE_PROMETHEUS_SCRAPE_SUPPORT
#define ENABLE_PROMETHEUS_SCRAPE_SUPPORT 0
#undef ENABLE_PROMETHEUS_PUSH
#define ENABLE_PROMETHEUS_PUSH 1
#endif

// WiFi options
// The hostname with which the esp can be reached.
// This value is used for both the actual hostname, as well as the mDNS hostname(mDNS names end with .local).
// While the normal hostname doesn't work with a static IP, mDNS still works.
// To define the host name using a macro pleases use ESPTHERM_HOSTNAME.
#ifdef ESPTHERM_HOSTNAME
static constexpr const char HOSTNAME[] = ESPTHERM_HOSTNAME;
#else
static constexpr const char HOSTNAME[] = "esp-wifi-thermometer";
#endif
// If set to an actual IP rather then IPADDR_ANY this will make the esp use that IP.
// However setting this to anything but IPADDR_ANY means the hostname wont work.
// If this is set to IPADDR_ANY the esp will get an IP address from the dhcp server.
// To configure this using a macro, please set ESPTHERM_STATIC_IP_ADDR.
#ifdef ESPTHERM_STATIC_IP_ADDR
static const IPAddress STATIC_IP = ESPTHERM_STATIC_IP_ADDR;
#else
static const IPAddress STATIC_IP = IPADDR_ANY;
#endif
// The WiFi gateway IP.
// Usually your router.
// Set to IPADDR_ANY to use the standard gateway of the WLAN.
// To configure this using a macro, please set ESPTHERM_GATEWAY_ADDR.
#ifdef ESPTHERM_GATEWAY_ADDR
static const IPAddress GATEWAY = ESPTHERM_GATEWAY_ADDR;
#else
static const IPAddress GATEWAY = IPADDR_ANY;
#endif
// The netmask of the subnet in which the esp is.
// Set to IPADDR_ANY to dynamically determine this when connecting to the WiFi access point.
// To configure this using a macro, please set ESPTHERM_SUBNET_MASK.
#ifdef ESPTHERM_SUBNET_MASK
static const IPAddress SUBNET = ESPTHERM_SUBNET_MASK;
#else
static const IPAddress SUBNET = IPAddress(255, 255, 255, 0);
#endif

// Web Server options
// Whether or not to enable the web server on the esp.
// Set to 1 to enable and to 0 to disable.
#ifndef ENABLE_WEB_SERVER
#define ENABLE_WEB_SERVER 1
#endif
#if ENABLE_WEB_SERVER == 1
// The port on which to open the web server that shows the measurements.
// The default web server port is 80.
static constexpr uint16_t WEB_SERVER_PORT = 80;
// The value for the Server header of all http responses sent by the webserver.
// The hardware name may be added to the server header.
// The default value is "ESP-WiFi-Thermometer".
#define SERVER_HEADER_PROGRAM "ESP-WiFi-Thermometer"
// Whether the hardware name in brackets should be added to the Server header.
// Set to 1 to enable and 0 to disable.
#define SERVER_HEADER_APPEND_HARDWARE 1
// The window size parameter used to decompress gzip files on the webserver.
// The window size used is pow(2, the absolute of the window size parameter).
// This window size has to be at least as big for decompression as it was for compression.
// Decompression requires a window size bytes large buffer on the esp.
// The range of valid values is -8 to -15.
// Default is -10.
static constexpr int8_t GZIP_DECOMP_WINDOW_SIZE = -10;

// Web server automatic config.
#if SERVER_HEADER_APPEND_HARDWARE != 1
static constexpr const char SERVER_HEADER[] = SERVER_HEADER_PROGRAM;
#else
#ifdef ESP32
static constexpr const char SERVER_HEADER[] = SERVER_HEADER_PROGRAM " (ESP32)";
#elif defined(ESP8266)
static constexpr const char SERVER_HEADER[] = SERVER_HEADER_PROGRAM " (ESP8266)";
#endif
#endif
#endif

// Sensor options
// Valid sensor types
#define SENSOR_TYPE_DHT 1
#define SENSOR_TYPE_DALLAS 2
// The type of sensor in use.
// Supported values: SENSOR_TYPE_DHT and SENSOR_TYPE_DALLAS.
#ifndef SENSOR_TYPE
#define SENSOR_TYPE SENSOR_TYPE_DHT
#endif
// The type of DHT sensor in use.
// Only used if SENSOR_TYPE is SENSOR_TYPE_DHT.
// Valid types are 11, 12, 21, and 22.
static constexpr uint8_t DHT_TYPE = 22;
// The gpio pin to which the data pin of the sensor is connected.
// Default is 5.
static constexpr uint8_t SENSOR_PIN = 5;

// Arduino OTA options
// Whether to enable the Arduino OTA server.
// Set to 1 to enable and to 0 to disable.
#ifndef ENABLE_ARDUINO_OTA
#define ENABLE_ARDUINO_OTA 1
#endif
// The port on which to open an Arduino OTA server.
// Uncomment to use a custom port.
// 3232 is the default for ESP32s.
// 8266 is the default for ESP8266s.
// #define ARDUINO_OTA_PORT 3232

// Prometheus options
// Whether to enable the prometheus scrape endpoint.
// This means publishing metrics on /metrics on the web server.
// Prometheus can then be configured to automatically read those values at a fixed interval.
// Set to 1 to enable and to 0 to disable.
#ifndef ENABLE_PROMETHEUS_SCRAPE_SUPPORT
#define ENABLE_PROMETHEUS_SCRAPE_SUPPORT 1
#endif
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
#if ENABLE_WEB_SERVER != 1
#undef ENABLE_PROMETHEUS_SCRAPE_SUPPORT
#define ENABLE_PROMETHEUS_SCRAPE_SUPPORT 0
#warning Prometheus scrape support requires the web server to be enabled.
#endif
#endif
// The namespace to be used as a prefix for most prometheus metrics.
// Will by default also be used as the pushgateway namespace.
// The default namespace is "esptherm".
static constexpr const char PROMETHEUS_NAMESPACE[] = "esptherm";
// The length of the prometheus namespace string.
static constexpr size_t PROMETHEUS_NAMESPACE_LEN = utility::strlen(PROMETHEUS_NAMESPACE);
// Whether the esp should automatically push measurements to a prometheus-pushgateway.
// This is done through HTTP post requests to a given address at fixed intervals.
// Set to 1 to enable and to 0 to disable.
#ifndef ENABLE_PROMETHEUS_PUSH
#define ENABLE_PROMETHEUS_PUSH 0
#endif
#if ENABLE_PROMETHEUS_PUSH == 1
// The address of the prometheus pushgateway to push the data to.
// Can be an IP address, a hostname, or a domain name.
static constexpr const char PROMETHEUS_PUSH_ADDR[] = "192.168.2.203";
// The port of the prometheus pushgateway.
// The default prometheus pushgateway port is 9091.
static constexpr uint16_t PROMETHEUS_PUSH_PORT = 9091;
// The time between two HTTP post requests sent to the pushgateway.
// Specified in seconds.
// Ignored in deep sleep mode.
// Prometheus default scrape interval is every 30 seconds.
static constexpr uint16_t PROMETHEUS_PUSH_INTERVAL = 30;
// The name of the job to use for the prometheus metrics when pushing.
// Leave empty to use the hostname of this device.
// The default is to use the device hostname.
static constexpr const char PROMETHEUS_PUSH_JOB[] = "";
// The length of the prometheus pushgateway job.
static constexpr size_t PROMETHEUS_PUSH_JOB_LEN = utility::strlen(PROMETHEUS_PUSH_JOB);
// The name of the instance to use for the prometheus metrics when pushing.
// Leave empty to use the device IP.
static constexpr const char PROMETHEUS_PUSH_INSTANCE[] = "";
// The length of the prometheus pushgateway instance string.
static constexpr size_t PROMETHEUS_PUSH_INSTANCE_LEN = utility::strlen(PROMETHEUS_PUSH_INSTANCE);
// The name of the namespace to use for the prometheus metrics when pushing.
// Leave empty to use the prometheus namespace.
// The default is to use the prometheus namespace.
static constexpr const char PROMETHEUS_PUSH_NAMESPACE[] = "";
// The length of the prometheus pushgateway namespace string.
static constexpr size_t PROMETHEUS_PUSH_NAMESPACE_LEN = utility::strlen(PROMETHEUS_PUSH_NAMESPACE);
#endif

// MQTT options
// Whether or not to enable the MQTT client.
// This will publish the measurements to the MQTT broker configured below.
// Set to 1 to enable and to 0 to disable.
#ifndef ENABLE_MQTT_PUBLISH
#define ENABLE_MQTT_PUBLISH 0
#endif
#if ENABLE_MQTT_PUBLISH == 1
// The address of the MQTT broker to publish the measurements to.
// Can be an IP address, a hostname, or a domain name.
static constexpr const char MQTT_BROKER_ADDR[] = "192.168.2.203";
// The port of the MQTT broker to publish to.
// The default MQTT broker port is 1883.
static constexpr uint16_t MQTT_BROKER_PORT = 1883;
// The time between two MQTT pushes.
// Specified in seconds.
// Ignored in deep sleep mode.
static constexpr uint16_t MQTT_PUBLISH_INTERVAL = 15;
// The namespace of the measurement topics.
// The MQTT topics published are structured like this: "namespace/measurement value".
// Also used as the name to open the MQTT connection.
// Leave empty to use the device hostname.
static constexpr const char MQTT_TOPIC_NAMESPACE[] = "";
// Whether MQTT publishing should be done anonymously.
// If enabled the MQTT connection will be initialized without a username or password.
// Set to 1 to enable and to 0 to disable.
#define MQTT_PUBLISH_ANONYMOUS 0
#endif

#endif /* SRC_CONFIG_H_ */
