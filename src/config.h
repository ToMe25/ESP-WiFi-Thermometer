/*
 * config.h
 *
 *  Created on: 24.12.2021
 *      Author: ToMe25
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <Arduino.h>

/**
 * This file contains a few variables and defines to be used as config values.
 */

// WiFi options
// The hostname with which the esp can be reached.
// This value is used for both the actual hostname, as well as the mDNS hostname(mDNS names end with .local).
// While the normal hostname doesn't work with a static IP, mDNS still works.
static constexpr char HOSTNAME[] = "esp32-dht22";
// The WiFi gateway IP.
// Set to INADDR_NONE to use the standard gateway of the WLAN.
// Usually your router.
static const IPAddress GATEWAY = INADDR_NONE;
// The netmask of the subnet in which the esp is.
// Set tp INADDR_NONE to dynamically determine this when connecting to the WiFi access point.
static const IPAddress SUBNET = INADDR_NONE;
// If set to an actual IP rather then INADDR_NONE this will make the esp use that IP.
// However setting this to anything but INADDR_NONE means the hostname wont work.
// If this is set to INADDR_NONE the esp will get an IP address from the dhcp server.
static const IPAddress STATIC_IP = INADDR_NONE;

// Web Server options
// The port on which to open the web server that shows the measurements.
// The default web server port is 80.
static const uint16_t WEB_SERVER_PORT = 80;

// DHT options
// The type of DHT sensor in use.
// Valid types are 11, 12, 21, and 22.
static const uint8_t DHT_TYPE = 22;
// The gpio pin to which the data pin of the DHT is connected.
static const uint8_t DHT_PIN = 5;

// Arduino OTA options
// Whether to enable the Arduino OTA server.
// Set to 1 to enable and to 0 to disable.
#define ENABLE_ARDUINO_OTA 1
// The port on which to open an Arduino OTA server.
// 3232 is the default for ESP32s.
static const uint16_t ARDUINO_OTA_PORT = 3232;

// Prometheus options
// Whether to enable the prometheus scrape endpoint.
// This means publishing metrics on /metrics on the web server.
// Prometheus can then be configured to automatically read those values at a fixed interval.
// Set to 1 to enable and to 0 to disable.
#define ENABLE_PROMETHEUS_SCRAPE_SUPPORT 1
// Whether the esp should automatically push measurements to a prometheus-pushgateway.
// This is done through HTTP post requests to a given address at fixed intervals.
// Set to 1 to enable and to 0 to disable.
#define ENABLE_PROMETHEUS_PUSH 0
// The address of the prometheus pushgateway to push the data to.
// Can be an IP address, a hostname, or a domain.
static constexpr char PROMETHEUS_PUSH_ADDR[] = "192.168.2.230";
// The port of the prometheus pushgateway.
static const uint16_t PROMETHEUS_PUSH_PORT = 9091;
// The time between two HTTP post requests sent to the pushgateway.
// Specified in seconds.
// Prometheus default scrape interval is every 30 seconds.
static const uint16_t PROMETHEUS_PUSH_INTERVAL = 30;
// The name of the job to use for the prometheus metrics when pushing.
// Leave empty to use the hostname of this device.
static constexpr char PROMETHEUS_PUSH_JOB[] = "";
// The name of the instance to use for the prometheus metrics when pushing.
// Leave empty to use the device IP.
static constexpr char PROMETHEUS_PUSH_INSTANCE[] = "";
// The name of the namespace to use for the prometheus metrics when pushing.
// Leave empty to not send namespace information.
static constexpr char PROMETHEUS_PUSH_NAMESPACE[] = "monitoring";

#endif /* SRC_CONFIG_H_ */
