/*
 * main.h
 *
 *  Created on: 16.11.2021
 *
 * Copyright (C) 2021 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <map>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

// includes the content of the file "wifissid.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char WIFI_SSID[] asm("_binary_wifissid_txt_start");
// includes the content of the file "wifipass.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char WIFI_PASS[] asm("_binary_wifipass_txt_start");
// includes the content of the file "otapass.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char OTA_PASS[] asm("_binary_otapass_txt_start");

extern const char INDEX_HTML[] asm("_binary_src_html_index_html_start");
extern const char MAIN_CSS[] asm("_binary_src_html_main_css_start");
extern const char INDEX_JS[] asm("_binary_src_html_index_js_start");
extern const char NOT_FOUND_HTML[] asm("_binary_src_html_not_found_html_start");

typedef std::function<uint16_t(AsyncWebServerRequest *request)> HTTPRequestHandler;

// WiFi variables
static constexpr char HOSTNAME[] = "esp32-dht22";
static const IPAddress GATEWAY(192, 168, 2, 1);
static IPAddress localhost = INADDR_NONE;
static IPv6Address localhost_ipv6;

// Web Server variables
static const uint16_t WEB_PORT = 80;
static AsyncWebServer server(WEB_PORT);

//DHT22 variables
static const uint8_t DHT_TYPE = DHT22;
static const uint8_t DHT_PIN = 5;

static DHT dht(DHT_PIN, DHT_TYPE);

static float temperature;
static float humidity;

// prometheus variables
static uint32_t used_heap;
static std::map<std::pair<std::string, uint16_t>, uint64_t> http_requests_total;

// Other variables
static std::string command;

static uint8_t loop_iterations = 0;

#endif /* SRC_MAIN_H_ */
