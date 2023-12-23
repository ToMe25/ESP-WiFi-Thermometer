/*
 * prometheus.h
 *
 *  Created on: 25.12.2021
 *
 * Copyright (C) 2021 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_PROMETHEUS_H_
#define SRC_PROMETHEUS_H_

#include "config.h"
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
#include "webhandler.h"
#endif
#if ENABLE_WEB_SERVER == 1 && (ENABLE_PROMETHEUS_PUSH == 1 || ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1)
#include <map>
#endif
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
extern std::map<String, std::map<std::pair<WebRequestMethod, uint16_t>, uint64_t>> http_requests_total;
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
 * @param openmetrics	Whether to generate OpenMetrics compliant output. Default is Prometheus 0.0.4 output.
 * @return	A string containing all the metrics for a prometheus server.
 */
String getMetrics(const bool openmetrics = false);

/**
 * Writes a metric entry constructed from the given values to the given buffer.
 * All the string arguments have to be compile time constant character arrays, **NOT POINTERS**!
 * This function does write a NUL byte after the text it writes, but this byte isn't considered for the return value.
 *
 * @tparam ns_l	The length of the metric namespace.
 * @tparam nm_l	The length of the metric name, without the unit and namespace part.
 * @tparam u_l	The length of the unit string.
 * @tparam dc_l	The length of the metric description.
 * @tparam tp_l	The length of the metric type.
 * @param buffer				The character buffer to write to.
 * @param metric_namespace		The name of the metric namespace to use.
 * @param metric_name			The name of the metric to write without the namespace and unit components.
 * @param metric_unit			The unit of the metric to write. May be empty.
 * @param metric_description	The description text for the metric.
 * @param metric_type			The metric type to write to the buffer.
 * @param value					The current value of the given metric.
 * @param openmetrics			Whether to use the OpenMetrics spec, instead of the default prometheus one.
 * @return	The number of bytes written to the output buffer.
 */
template<size_t ns_l, size_t nm_l, size_t u_l, size_t dc_l, size_t tp_l>
size_t writeMetric(char *buffer, const char (&metric_namespace)[ns_l],
		const char (&metric_name)[nm_l], const char (&metric_unit)[u_l],
		const char (&metric_description)[dc_l], const char (&metric_type)[tp_l],
		const double value, const bool openmetrics = false);

/**
 * Writes a metric metadata line constructed from the given strings to the given buffer.
 * All the string arguments have to be compile time constant character arrays, **NOT POINTERS**!
 * This function does write a NUL byte after the text it writes, but this byte isn't considered for the return value.
 *
 * @tparam fnm_l	The length of the metadata field name.
 * @tparam ns_l		The length of the metric namespace.
 * @tparam nm_l		The length of the metric name, without the unit and namespace part.
 * @tparam u_l		The length of the unit string.
 * @tparam vl_l		The length of the value string.
 * @param buffer				The character buffer to write to.
 * @param field_name			The name of the metric metadata field. Has to be in CAPS.
 * @param metric_namespace		The name of the metric namespace to use.
 * @param metric_name			The name of the metric to write without the namespace and unit components.
 * @param metric_unit			The unit of the metric to write. May be empty.
 * @param value					The value string to write to the output buffer.
 * @return	The number of characters that were written to the output buffer.
 */
template<size_t fnm_l, size_t ns_l, size_t nm_l, size_t u_l, size_t vl_l>
size_t writeMetricMetadataLine(char *buffer, const char (&field_name)[fnm_l],
		const char (&metric_namespace)[ns_l], const char (&metric_name)[nm_l],
		const char (&metric_unit)[u_l], const char (&value)[vl_l]);
#endif

#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
/**
 * Checks whether the given Accept header accepts openmetrics text protocol version 1.0.0.
 *
 * @param accept_str	The Accept header to check.
 * @return	True if the given header contains "application/openmetrics-text".
 */
bool acceptsOpenMetrics(const char *accept_str);

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
