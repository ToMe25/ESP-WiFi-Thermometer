/*
 * prometheus.cpp
 *
 *  Created on: 25.12.2021
 *      Author: ToMe25
 */

#include "prometheus.h"
#include "main.h"
#include <iomanip>
#include <iostream>
#include <sstream>

uint32_t prom::used_heap = 0;
std::map<std::pair<std::string, uint16_t>, uint64_t> prom::http_requests_total;
std::chrono::time_point<std::chrono::system_clock> prom::last_push;
HTTPClient prom::http;
std::string prom::push_url;

void prom::setup() {
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
	registerRequestHandler("/metrics", HTTP_GET, handleMetrics);
#endif
}

void prom::loop() {
	used_heap = ESP.getHeapSize() - ESP.getFreeHeap();

#if ENABLE_PROMETHEUS_PUSH == 1
	pushMetrics();
#endif
}

void prom::connect() {
#if ENABLE_PROMETHEUS_PUSH == 1
	std::ostringstream stream;
	stream << "http://" << PROMETHEUS_PUSH_ADDR;
	stream << ':' << PROMETHEUS_PUSH_PORT;

	stream << "/metrics/job/";
	if (strlen(PROMETHEUS_PUSH_JOB) > 0) {
		stream << PROMETHEUS_PUSH_JOB;
	} else {
		stream << HOSTNAME;
	}
	stream << "/instance/";
	if (strlen(PROMETHEUS_PUSH_INSTANCE) > 0) {
		stream << PROMETHEUS_PUSH_INSTANCE;
	} else {
		stream << localhost.toString().c_str();
	}
	if (strlen(PROMETHEUS_PUSH_NAMESPACE) > 0) {
		stream << "/namespace/";
		stream << PROMETHEUS_PUSH_NAMESPACE;
	}

	push_url = stream.str();
#endif
}

std::string prom::getMetrics() {
	std::ostringstream metrics;
	metrics << "# HELP environment_temperature The current external temperature measured using a DHT22."
			<< std::endl;
	metrics << "# TYPE environment_temperature gauge" << std::endl;
	metrics << "environment_temperature " << std::setprecision(3) << temperature << std::endl;

	metrics << "# HELP environment_humidity The current external relative humidity measured using a DHT22."
			<< std::endl;
	metrics << "# TYPE environment_humidity gauge" << std::endl;
	metrics << "environment_humidity " << std::setprecision(3) << humidity << std::endl;

	metrics << "# HELP process_heap_bytes The amount of heap used on the ESP32 in bytes."
			<< std::endl;
	metrics << "# TYPE process_heap_bytes gauge" << std::endl;
	metrics << "process_heap_bytes " << used_heap << std::endl;

	metrics << "# HELP http_requests_total The total number of http requests handled by this server."
			<< std::endl;
	metrics << "# TYPE http_requests_total counter" << std::endl;

	for (std::pair<std::pair<std::string, uint16_t>, uint64_t> element : http_requests_total) {
		metrics << "http_requests_total{method=\"get\",code=\""
				<< element.first.second << "\",path=\"" << element.first.first
				<< "\"} " << element.second << std::endl;
	}

	return metrics.str();
}

#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
uint16_t prom::handleMetrics(AsyncWebServerRequest *request) {
	request->send(200, "text/plain", getMetrics().c_str());
	return 200;
}
#endif

#if ENABLE_PROMETHEUS_PUSH == 1
void prom::pushMetrics() {
	if (!WiFi.isConnected()) {
		return;
	}

	using std::chrono::time_point;
	using std::chrono::system_clock;
	time_point<system_clock> now = system_clock::now();

	if (std::chrono::duration_cast<std::chrono::seconds>(now - last_push).count() >= PROMETHEUS_PUSH_INTERVAL) {
		last_push = now;
		http.begin(push_url.c_str());
		int code = http.POST(getMetrics().c_str());
		http.end();
		if (code != 200) {
			Serial.print("Received http status code ");
			Serial.print(code);
			Serial.print(" when trying to push metrics.");
		}
	}
}
#endif
