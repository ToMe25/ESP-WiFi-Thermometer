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

#include "config.h"
#include <chrono>
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
static IPAddress localhost = STATIC_IP;
static IPv6Address localhost_ipv6;

// Web Server variables
static AsyncWebServer server(WEB_SERVER_PORT);

//DHT22 variables
static DHT dht(DHT_PIN, DHT_TYPE);

static float temperature;
static float humidity;
static std::chrono::time_point<std::chrono::system_clock> last_measurement;

// prometheus variables
static uint32_t used_heap;
static std::map<std::pair<std::string, uint16_t>, uint64_t> http_requests_total;

// Other variables
static std::string command;

static uint8_t loop_iterations = 0;

// Methods
/*
 * Returns a string with the time since the last measurement formatted like this "Hour(24):Minute:Second.Millisecond".
 */
std::string getTimeSinceMeasurement();

/*
 * Registers the given handler for the web server, and increments the web requests counter
 * by one each time it is called.
 */
void registerRequestHandler(const char *uri, WebRequestMethodComposite method,
		HTTPRequestHandler handler);

/*
 * Registers a request handler that returns the given content type and web page each time it is called.
 * Expects request type get.
 * Also increments the request counter.
 */
void registerStaticHandler(const char *uri, const char *content_type,
		const char *page);

#endif /* SRC_MAIN_H_ */
