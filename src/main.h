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

// Includes the content of the file "wifissid.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char WIFI_SSID[] asm("_binary_wifissid_txt_start");
// Includes the content of the file "wifipass.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char WIFI_PASS[] asm("_binary_wifipass_txt_start");
// Includes the content of the file "otapass.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char OTA_PASS[] asm("_binary_otapass_txt_start");

// WiFi variables
extern IPAddress localhost;
#ifdef ESP32
extern IPv6Address localhost_ipv6;
#endif

// Other variables
extern std::string command;

extern uint8_t loop_iterations;

extern volatile uint64_t start_ms;

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

#ifdef ESP32
/**
 * The function handling the WiFi events that may occur.
 *
 * @param id	The id of the WiFi event.
 * @param info	Info about the WiFi event.
 */
void onWiFiEvent(WiFiEventId_t id, WiFiEventInfo_t info);
#elif defined(ESP8266)
/**
 * The function handling the WiFi events that may occur.
 *
 * @param id	The id of the WiFi event.
 */
void onWiFiEvent(WiFiEvent_t id);
#endif

/**
 * The core of this program, the method that gets called repeatedly as long as the program runs.
 */
void loop();

/**
 * Responds to serial input by executing actions and printing a response.
 *
 * @return	True if the input string was a valid command.
 */
bool handle_serial_input(const std::string &input);

/**
 * Print the given temperature in degrees celsius and degrees fahrenheit.
 *
 * @param out	The print object to print to.
 * @param temp	The temperature to print. In degrees celsius.
 */
void printTemperature(Print &out, const float temp);

#endif /* SRC_MAIN_H_ */
