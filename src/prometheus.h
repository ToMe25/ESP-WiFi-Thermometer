/*
 * prometheus.h
 *
 *  Created on: 25.12.2021
 *      Author: ToMe25
 */

#ifndef SRC_PROMETHEUS_H_
#define SRC_PROMETHEUS_H_

#include "config.h"
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
#include "webhandler.h"
#endif
#include <map>
#if ENABLE_PROMETHEUS_PUSH == 1
#ifdef ESP32
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif
#endif
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
#include <ESPAsyncWebServer.h>
#endif

/**
 * This header, and the source file with the same name, contain everything for the prometheus integration.
 */
namespace prom {
#ifdef ESP32// From what I could find this seems to be impossible on a ESP8266.
extern uint32_t used_heap;
#endif
#if ENABLE_WEB_SERVER == 1 && (ENABLE_PROMETHEUS_PUSH == 1 || ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1)
extern std::map<std::pair<String, uint16_t>, uint64_t> http_requests_total;
#endif
#if ENABLE_PROMETHEUS_PUSH == 1
#if ENABLE_DEEP_SLEEP_MODE != 1
extern uint64_t last_push;
#endif
extern AsyncClient *tcpClient;
extern std::string push_url;
#endif

/**
 * Initializes the prometheus integration.
 */
void setup();

/**
 * A method that does everything that should be done every loop iteration.
 */
void loop();

/**
 * A handler for things that should happen when a new WiFi connection is established.
 */
void connect();

#if (ENABLE_PROMETHEUS_PUSH == 1 || ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1)
/**
 * Creates a string containing the metrics for prometheus.
 *
 * @return	A string containing all the metrics for a prometheus server.
 */
std::string getMetrics();
#endif

#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
/**
 * The callback method to respond to a HTTP get request for the metrics page.
 *
 * @param request	The request to respond to.
 * @return	The HTTP status code of the response.
 */
web::ResponseData handleMetrics(AsyncWebServerRequest *request);
#endif

#if ENABLE_PROMETHEUS_PUSH == 1
/**
 * This method pushes the prometheus metrics to the configured prometheus pushgateway server.
 */
void pushMetrics();
#endif
}

#endif /* SRC_PROMETHEUS_H_ */
