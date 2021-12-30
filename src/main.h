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
#include <ESPAsyncWebServer.h>
// It would be possible to always only include one, but that makes it a pain when switching between them.
// Especially when doing it repeatedly for testing.
#include <DallasTemperature.h>
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
extern const uint8_t FAVICON_ICO_START[] asm("_binary_src_html_favicon_ico_start");
extern const uint8_t FAVICON_ICO_END[] asm("_binary_src_html_favicon_ico_end");

typedef std::function<uint16_t(AsyncWebServerRequest *request)> HTTPRequestHandler;

// WiFi variables
extern IPAddress localhost;
extern IPv6Address localhost_ipv6;

// Web Server variables
extern AsyncWebServer server;

//Sensor variables
#if SENSOR_TYPE == SENSOR_TYPE_DHT
extern DHT dht;
#elif SENSOR_TYPE == SENSOR_TYPE_DALLAS
extern OneWire wire;
extern DallasTemperature sensors;
#endif

extern float temperature;
extern float humidity;
extern uint64_t last_measurement;

// Other variables
extern std::string command;

extern uint8_t loop_iterations;

extern uint64_t start;

// Methods
/**
 * Initializes the program and everything needed by it.
 */
void setup();

/**
 * Initializes everything related to WiFi, and establishes a connection to an WiFi access point, if possible.
 */
void setupWiFi();

#if ENABLE_ARDUINO_OTA == 1
/**
 * Initializes everything required for Arduino OTA.
 */
void setupOTA();
#endif

#if ENABLE_WEB_SERVER == 1
/**
 * Initializes the Web Server and the mDNS entry for the web server.
 */
void setupWebServer();
#endif

/**
 * The core of this program, the method that gets called repeatedly as long as the program runs.
 */
void loop();

/**
 * Reads in the sensor measurements and stores them in the correct values.
 */
void measure();

#if ENABLE_WEB_SERVER == 1
/**
 * The request handler for / and /index.html.
 *
 * @param request	The request to respond to.
 * @return	The response http status code.
 */
uint16_t getIndex(AsyncWebServerRequest *request);

/**
 * The request handler for /data.json.
 * Responds with a json object containing the current temperature and humidity,
 * as well as the time since the last measurement.
 *
 * @param request	The web request to handle.
 * @return	The returned http status code.
 */
uint16_t getJson(AsyncWebServerRequest *request);

/**
 * Returns a string with the time since the last measurement formatted like this "Hour(24):Minute:Second.Millisecond".
 * Currently only used for the web server.
 *
 * @return	A formatted string representing the time since the last measurement.
 */
std::string getTimeSinceMeasurement();

/**
 * Registers the given handler for the web server, and increments the web requests counter
 * by one each time it is called.
 */
void registerRequestHandler(const char *uri, WebRequestMethodComposite method,
		HTTPRequestHandler handler);

/**
 * Registers a request handler that returns the given content type and web page each time it is called.
 * Expects request type get.
 * Also increments the request counter.
 */
void registerStaticHandler(const char *uri, const char *content_type,
		const char *page);
#endif /* ENABLE_WEB_SERVER */

/**
 * Converts the given temperature from degrees celsius to degrees fahrenheit.
 *
 * @param celsius	The temperature to convert in celsius.
 * @return	The converted temperature in fahrenheit.
 */
float celsiusToFahrenheit(float celsius);

#endif /* SRC_MAIN_H_ */
