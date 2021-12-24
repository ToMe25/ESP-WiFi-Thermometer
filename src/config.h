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

// WIFI options
static constexpr char HOSTNAME[] = "esp32-dht22";
// The WiFi gateway IP.
// Set to INADDR_NONE to use the standard gateway of the WLAN.
// Usually your router.
static const IPAddress GATEWAY = INADDR_NONE;
static const IPAddress SUBNET = INADDR_NONE;
// If set to an actual IP rather then INADDR_NONE this will make the esp use that IP.
// However setting this means the hostname wont work.
static const IPAddress STATIC_IP = INADDR_NONE;

// Web Server options
static const uint16_t WEB_SERVER_PORT = 80;

// DHT options
static const uint8_t DHT_TYPE = 22;
static const uint8_t DHT_PIN = 5;

// Prometheus options
// Comment out to disable the prometheus metrics page.
#define ENABLE_PROMETHEUS_SUPPORT

#endif /* SRC_CONFIG_H_ */
