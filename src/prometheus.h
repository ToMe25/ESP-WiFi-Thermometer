/*
 * prometheus.h
 *
 *  Created on: 25.12.2021
 *      Author: ToMe25
 */

#ifndef SRC_PROMETHEUS_H_
#define SRC_PROMETHEUS_H_

#include "config.h"
#include <map>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>

/**
 * This header, and the source file with the same name, contain everything for the prometheus integration.
 */
namespace prom {
extern uint32_t used_heap;
extern std::map<std::pair<std::string, uint16_t>, uint64_t> http_requests_total;
extern uint64_t last_push;
extern TaskHandle_t metrics_pusher;
extern HTTPClient http;
extern std::string push_url;

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

/**
 * Creates a string containing the metrics for prometheus.
 *
 * @return	A string containing all the metrics for a prometheus server.
 */
std::string getMetrics();

#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
/**
 * The callback method to respond to a HTTP get request for the metrics page.
 *
 * @param request	The request to respond to.
 * @return	The HTTP status code of the response.
 */
uint16_t handleMetrics(AsyncWebServerRequest *request);
#endif

#if ENABLE_PROMETHEUS_PUSH == 1
/**
 * This is the method run in the Metric Pusher task.
 * Should run forever delaying for as many seconds as specified in PROMETHEUS_PUSH_INTERVAL between pushMetrics calls.
 *
 * @param param	The task parameters. Should be empty.
 */
void metricPusher(void *param);

/**
 * This method pushes the prometheus metrics to the configured prometheus pushgateway server.
 */
void pushMetrics();
#endif
}

#endif /* SRC_PROMETHEUS_H_ */
