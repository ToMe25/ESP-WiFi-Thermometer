/*
 * prometheus.cpp
 *
 *  Created on: 25.12.2021
 *      Author: ToMe25
 */

#include "prometheus.h"
#include "main.h"
#include <iomanip>
#include <sstream>

#ifdef ESP32// From what I could find this seems to be impossible on a ESP8266.
uint32_t prom::used_heap = 0;
#endif
std::map<std::pair<std::string, uint16_t>, uint64_t> prom::http_requests_total;
#if (ENABLE_PROMETHEUS_PUSH == 1 && ENABLE_DEEP_SLEEP_MODE != 1)
uint64_t prom::last_push = 0;
#endif
AsyncClient *prom::tcpClient = NULL;
std::string prom::push_url;

void prom::setup() {
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
	registerRequestHandler("/metrics", HTTP_GET, handleMetrics);
#endif
}

void prom::loop() {
#ifdef ESP32// From what I could find this seems to be impossible on a ESP8266.
	used_heap = ESP.getHeapSize() - ESP.getFreeHeap();
#endif

#if ENABLE_PROMETHEUS_PUSH == 1
	pushMetrics();
#endif
}

void prom::connect() {
#if ENABLE_PROMETHEUS_PUSH == 1
	std::ostringstream stream;
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
	metrics << "# HELP environment_temperature The current measured external temperature in degrees celsius."
			<< std::endl;
	metrics << "# TYPE environment_temperature gauge" << std::endl;
	metrics << "environment_temperature " << std::setprecision(3) << temperature << std::endl;

	metrics << "# HELP environment_humidity The current measured external relative humidity in percent."
			<< std::endl;
	metrics << "# TYPE environment_humidity gauge" << std::endl;
	metrics << "environment_humidity " << std::setprecision(3) << humidity << std::endl;

#ifdef ESP32// From what I could find this seems to be impossible on a ESP8266.
	metrics << "# HELP process_heap_bytes The amount of heap used on the ESP in bytes."
			<< std::endl;
	metrics << "# TYPE process_heap_bytes gauge" << std::endl;
	metrics << "process_heap_bytes " << used_heap << std::endl;
#endif

#if ENABLE_WEB_SERVER == 1
	metrics << "# HELP http_requests_total The total number of http requests handled by this server."
			<< std::endl;
	metrics << "# TYPE http_requests_total counter" << std::endl;

	for (std::pair<std::pair<std::string, uint16_t>, uint64_t> element : http_requests_total) {
		metrics << "http_requests_total{method=\"get\",code=\""
				<< element.first.second << "\",path=\"" << element.first.first
				<< "\"} " << element.second << std::endl;
	}
#endif

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

#if ENABLE_DEEP_SLEEP_MODE != 1
	uint64_t now = millis();

	if (now - last_push >= PROMETHEUS_PUSH_INTERVAL * 1000) {
		last_push = now;
#endif

		if (tcpClient != NULL) {
			Serial.println(F("Closing already open push connection!"));
			tcpClient->close(true);
			if (tcpClient != NULL) {
				AsyncClient *cli = tcpClient;
				tcpClient = NULL;
				delete cli;
			}
		}

		tcpClient = new AsyncClient();
		tcpClient->setAckTimeout(30);
		tcpClient->setRxTimeout(30);
		tcpClient->onError([](void *arg, AsyncClient *cli, int error) {
			Serial.println(F("Connecting to the metrics server failed!"));
			Serial.print(F("Connection Error: "));
			Serial.println(error);
			tcpClient = NULL;
			delete cli;
		}, NULL);

		tcpClient->onConnect([](void *arg, AsyncClient *cli) {
			cli->onDisconnect([](void *arg, AsyncClient *c) {
				if (tcpClient != NULL) {
					Serial.println(F("Connection to prometheus pushgateway server was closed while reading or writing."));
					tcpClient = NULL;
					delete c;
				}
			}, NULL);

			size_t *read = new size_t;
			*read = 0;
			cli->onData([read](void *arg, AsyncClient *c, void *data, size_t len) mutable {
				uint8_t *d = (uint8_t*) data;

				for (size_t i = 0; i < len; i++) {
					if (*read > 8 && isDigit(d[i])) {
						char status_code[4] { 0 };
						for (uint8_t j = 0; j < 3; j++) {
							status_code[j] = d[i + j];
						}

						uint32_t code = atoi(status_code);
						if (code != 200) {
							Serial.print(F("Received http status code "));
							Serial.print(code);
							Serial.println(F(" when trying to push metrics."));
						}

						tcpClient = NULL;
						c->close(true);
						delete c;
						delete read;
						return;
					}
					(*read)++;
				}
			}, NULL);

			cli->write("POST ");
			cli->write(push_url.c_str());
			cli->write(" HTTP/1.0\r\nHost: ");
			cli->write(PROMETHEUS_PUSH_ADDR);
			cli->write("\r\n");
			std::string metrics = getMetrics();
			cli->write("Content-Type: application/x-www-form-urlencoded\r\n");
			cli->write("Content-Length: ");
			std::ostringstream converter;
			converter << metrics.length();
			cli->write(converter.str().c_str());
			cli->write("\r\n\r\n");
			cli->write(metrics.c_str());
			cli->write("\r\n\r\n");
		}, NULL);

		if (!tcpClient->connect(PROMETHEUS_PUSH_ADDR, PROMETHEUS_PUSH_PORT)) {
			Serial.println(F("Connecting to the metrics server failed!"));
			if (tcpClient != NULL) {
				AsyncClient *cli = tcpClient;
				tcpClient = NULL;
				delete cli;
			}
		}
#if ENABLE_DEEP_SLEEP_MODE == 1
		while (tcpClient != NULL) {
			delay(10);
		}
#else
	}
#endif
}
#endif
