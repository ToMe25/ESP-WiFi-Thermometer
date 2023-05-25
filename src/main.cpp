/*
 * main.h
 *
 *  Created on: 06.11.2020
 *
 * Copyright (C) 2020-2021 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include "main.h"
#include "prometheus.h"
#include "mqtt.h"
#include <iomanip>
#if ENABLE_WEB_SERVER == 1
#include <sstream>
#include <uzlib.h>
#endif
#include <Adafruit_Sensor.h>
#if ENABLE_ARDUINO_OTA == 1
#include <ArduinoOTA.h>
#endif
#ifdef ESP32
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

IPAddress localhost;
#ifdef ESP32
IPv6Address localhost_ipv6;
#endif
#if ENABLE_WEB_SERVER == 1
AsyncWebServer server(WEB_SERVER_PORT);
#endif
#if SENSOR_TYPE == SENSOR_TYPE_DHT
DHT dht(SENSOR_PIN, DHT_TYPE);
#elif SENSOR_TYPE == SENSOR_TYPE_DALLAS
OneWire wire(SENSOR_PIN);
DallasTemperature sensors(&wire);
#endif
float temperature = 0;
float humidity = 0;
uint64_t last_measurement = 0;
std::string command;
uint8_t loop_iterations = 0;
uint64_t start_ms = 0;

void setup() {
	start_ms = millis();
	Serial.begin(115200);

#if SENSOR_TYPE == SENSOR_TYPE_DHT
	dht.begin();
#elif SENSOR_TYPE == SENSOR_TYPE_DALLAS
	sensors.begin();
#endif

	setupWiFi();
#if ENABLE_ARDUINO_OTA == 1
	setupOTA();
#endif

#if ENABLE_WEB_SERVER == 1
	setupWebServer();
#endif

	prom::setup();
	mqtt::setup();

#if ENABLE_DEEP_SLEEP_MODE == 1
	measure();

	printTemperature(Serial, temperature);
	Serial.print("Humidity: ");
	Serial.print(humidity);
	Serial.println('%');

	if (WiFi.waitForConnectResult() == WL_CONNECTED) {
#if ENABLE_PROMETHEUS_PUSH == 1
		prom::pushMetrics();
#endif
#if ENABLE_MQTT_PUBLISH == 1
		mqtt::publishMeasurements();
#endif
	} else {
		Serial.println("Failed to connect to WiFi!");
	}

	WiFi.disconnect(1);

#ifdef ESP32
	esp_sleep_enable_timer_wakeup(DEEP_SLEEP_MODE_MEASUREMENT_INTERVAL * 1000000 - (micros() - start_ms * 1000));
	esp_deep_sleep_start();
#elif defined(ESP8266)
	ESP.deepSleep(DEEP_SLEEP_MODE_MEASUREMENT_INTERVAL * 1000000 - (micros() - start_ms * 1000));
#endif
#endif /* ENABLE_DEEP_SLEEP_MODE */
}

void setupWiFi() {
	WiFi.mode(WIFI_STA);
#if ENABLE_DEEP_SLEEP_MODE != 1
	WiFi.disconnect(1);
#endif
	WiFi.onEvent(onWiFiEvent);

	if (STATIC_IP != IPADDR_ANY || GATEWAY != IPADDR_ANY || SUBNET != IPADDR_ANY) {
		if (!WiFi.config(STATIC_IP, GATEWAY, SUBNET)) {
			Serial.println("Configuring WiFi failed!");
			return;
		}

		localhost = STATIC_IP;
	}

	WiFi.begin(WIFI_SSID, WIFI_PASS);
}

#if ENABLE_ARDUINO_OTA == 1
void setupOTA() {
	ArduinoOTA.setHostname(HOSTNAME);
#ifdef ARDUINO_OTA_PORT
	ArduinoOTA.setPort(ARDUINO_OTA_PORT);
#endif
	ArduinoOTA.setPassword(OTA_PASS);

	ArduinoOTA.onStart([]() {
		Serial.println("Start updating sketch.");
	});

	ArduinoOTA.onProgress([](uint progress, uint total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});

	ArduinoOTA.onEnd([]() {
		Serial.println("\nUpdate Done.");
	});

	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("OTA Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			Serial.println("Auth Failed.");
		} else if (error == OTA_BEGIN_ERROR) {
			Serial.println("Begin Failed.");
		} else if (error == OTA_CONNECT_ERROR) {
			Serial.println("Connect Failed.");
		} else if (error == OTA_RECEIVE_ERROR) {
			Serial.println("Receive Failed.");
		} else if (error == OTA_END_ERROR) {
			Serial.println("End Failed.");
		}
	});

	ArduinoOTA.begin();
}
#endif /* ENABLE_ARDUINO_OTA */

#if ENABLE_WEB_SERVER == 1
void setupWebServer() {
	registerProcessedStaticHandler("/", "text/html", INDEX_HTML, processIndexTemplates);
	registerProcessedStaticHandler("/index.html", "text/html", INDEX_HTML, processIndexTemplates);

	registerCompressedStaticHandler("/main.css", "text/css", MAIN_CSS_START, MAIN_CSS_END);
	registerCompressedStaticHandler("/index.js", "text/javascript", INDEX_JS_START, INDEX_JS_END);

	registerRequestHandler("/temperature", HTTP_GET,
			[](AsyncWebServerRequest *request) -> uint16_t {
				std::ostringstream os;
				os << std::setprecision(3) << temperature;
				request->send(200, "text/plain", os.str().c_str());
				return 200;
			});

	registerRequestHandler("/humidity", HTTP_GET,
			[](AsyncWebServerRequest *request) -> uint16_t {
				std::ostringstream os;
				os << std::setprecision(3) << humidity;
				request->send(200, "text/plain", os.str().c_str());
				return 200;
			});

	registerRequestHandler("/data.json", HTTP_GET, getJson);

	registerCompressedStaticHandler("/favicon.ico", "image/x-icon", FAVICON_ICO_GZ_START,
			FAVICON_ICO_GZ_END);
	registerCompressedStaticHandler("/favicon.png", "image/png", FAVICON_PNG_GZ_START,
			FAVICON_PNG_GZ_END);
	registerCompressedStaticHandler("/favicon.svg", "image/svg+xml", FAVICON_SVG_GZ_START,
			FAVICON_SVG_GZ_END);

	server.onNotFound(
			[](AsyncWebServerRequest *request) {
				request->send(404, "text/html", NOT_FOUND_HTML);
				Serial.print(F("A client tried to access the not existing file \""));
				Serial.print(request->url().c_str());
				Serial.println("\".");
				prom::http_requests_total[std::pair<String, uint16_t>(
						request->url(), 404)]++;
			});

	DefaultHeaders::Instance().addHeader("Server", SERVER_HEADER);
	server.begin();

	uzlib_init();

#if ENABLE_ARDUINO_OTA != 1
	MDNS.begin(HOSTNAME);
#endif

	MDNS.addService("http", "tcp", WEB_SERVER_PORT);
}

size_t getGzipDecompressedSize(const uint8_t *end_ptr) {
	size_t dlen = end_ptr[-1];
	dlen = 256 * dlen + end_ptr[-2];
	dlen = 256 * dlen + end_ptr[-3];
	dlen = 256 * dlen + end_ptr[-4];
	return dlen;
}

String processIndexTemplates(const String &temp) {
	if (temp == "TEMP") {
		std::ostringstream converter;
		converter << std::setprecision(3) << temperature;
		return converter.str().c_str();
	} else if (temp == "HUMID") {
		std::ostringstream converter;
		converter << std::setprecision(3) << humidity;
		return converter.str().c_str();
	} else if (temp == "TIME") {
		return getTimeSinceMeasurement().c_str();
	} else {
		return "Error";
	}
}

uint16_t getJson(AsyncWebServerRequest *request) {
	std::ostringstream json;
	json << "{\"temperature\": ";
	json << std::setprecision(3) << temperature;
	json << ", \"humidity\": ";
	json << std::setprecision(3) << humidity;
	json << ", \"time\": \"";
	json << getTimeSinceMeasurement();
	json << "\"}";
	AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json.str().c_str());
	response->addHeader("Cache-Control", "no-cache");
	request->send(response);
	return 200;
}
#endif /* ENABLE_WEB_SERVER */

#ifdef ESP32
void onWiFiEvent(WiFiEventId_t id, WiFiEventInfo_t info) {
	switch (id) {
	case SYSTEM_EVENT_STA_START:
		WiFi.setHostname(HOSTNAME);
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		WiFi.enableIpV6();

		if (STATIC_IP != IPADDR_ANY) {
			Serial.print("WiFi ready ");
			Serial.print(millis() - start_ms);
			Serial.println("ms after start.");
			Serial.print("STA IP: ");
			Serial.println(localhost = WiFi.localIP());
			prom::connect();
			mqtt::connect();
		}
		break;
	case SYSTEM_EVENT_GOT_IP6:
		Serial.print("STA IPv6: ");
		Serial.println(localhost_ipv6 = WiFi.localIPv6());
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
#if CORE_DEBUG_LEVEL == 5
		delay(10);// if not doing this the additional logging causes the next log entry to not work.
#endif
		Serial.print("WiFi ready ");
		Serial.print(millis() - start_ms);
		Serial.println("ms after start.");
		Serial.print("STA IP: ");
		Serial.println(localhost = WiFi.localIP());
		prom::connect();
		mqtt::connect();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		WiFi.reconnect();
		break;
	case SYSTEM_EVENT_SCAN_DONE:
		Serial.println("WiFi scan results: ");
		Serial.print("Found ");
		Serial.print((uint16_t) info.scan_done.number);
		Serial.println(" WiFi networks.");
		for (uint8_t i = 0; i < info.scan_done.number; i++) {
			String SSID;
			uint8_t encryptionType;
			int32_t RSSI;
			uint8_t *BSSID;
			int32_t channel;
			WiFi.getNetworkInfo(i, SSID, encryptionType, RSSI, BSSID, channel);
			Serial.print("network ");
			Serial.print(i + 1);
			Serial.print(": ssid = ");
			Serial.print(SSID);
			Serial.print(", rssi = ");
			Serial.print(RSSI);
			Serial.print(", channel = ");
			Serial.print(channel);
			Serial.print(", encryptionType = ");
			Serial.println(encryptionType);
		}
		break;
	default:
		break;
	}
}
#elif defined(ESP8266)
void onWiFiEvent(WiFiEvent_t id) {
	switch (id) {
	case WIFI_EVENT_STAMODE_GOT_IP:
		Serial.print("WiFi ready ");
		Serial.print(millis() - start_ms);
		Serial.println("ms after start.");
		Serial.print("STA IP: ");
		Serial.println(localhost = WiFi.localIP());
		prom::connect();
		mqtt::connect();
		break;
	case WIFI_EVENT_STAMODE_DISCONNECTED:
		WiFi.reconnect();
		break;
	default:
		break;
	}
}
#endif

void loop() {
	uint64_t start = millis();

	if (loop_iterations % 4 == 0) {
		measure();

		if (loop_iterations % 20 == 0 && millis() - last_measurement < 10000) {
			printTemperature(Serial, temperature);
			if (humidity != 0) {
				Serial.print("Humidity: ");
				Serial.print(humidity);
				Serial.println('%');
			}
		}
	}

	uint available = Serial.available();
	if (available > 0) {
		char input[available];
		Serial.readBytes(input, available);

		for (char c : input) {
			if (c == '\b') {
				Serial.print("\b \b");
				if (!command.empty()) {
					command.pop_back();
				}
			} else if (c == '\n') {
				if (!command.empty() && !handle_serial_input(command)) {
					Serial.println();
					Serial.print("Unknown Command: ");
					Serial.println(command.c_str());
					Serial.println("Use \"help\" to get a list of valid commands.");
				}
				command = "";
			} else if (!iscntrl(c)) {
				Serial.write(c);
				command += c;
			}
		}

		if (handle_serial_input(command)) {
			command = "";
		}
	}

	if (loop_iterations == 200) {
		loop_iterations = 0;
	}

#if ENABLE_ARDUINO_OTA == 1
	ArduinoOTA.handle();
#endif

	prom::loop();
	mqtt::loop();

	loop_iterations++;
	uint64_t end = millis();
	delay(max(0, 500 - int16_t(end - start)));
}

bool handle_serial_input(const std::string &input) {
	if (input == "temperature" || input == "temp") {
		Serial.println();
		printTemperature(Serial, temperature);
		return true;
	} else if (input == "humidity") {
		Serial.println();
		Serial.print("Relative humidity: ");
		Serial.print(humidity);
		Serial.println('%');
		return true;
	} else if (input == "ip") {
		Serial.println();
		Serial.println("IP Address: ");
#ifdef ESP32
		Serial.print("IPv6: ");
		Serial.println(localhost_ipv6);
#endif
		Serial.print("IPv4: ");
		Serial.println(localhost);
		return true;
	} else if (input == "scan") {
		Serial.println();
#ifdef ESP32
		Serial.println("Starting WiFi scan...");
		WiFi.scanNetworks(true, true);
#elif defined(ESP8266)
		Serial.println(F("WiFi scanning is not currently supported on ESP8266 hardware."));
#endif
		return true;
	} else if (input == "help") {
		Serial.println();
		Serial.println(F("ESP-WiFi-Thermometer help:"));
		Serial.println(F("temperature (or temp): Prints the last measured temperature in °C and °F."));
		Serial.println(F("humidity:              Prints the relative humidity in %."));
		Serial.println(F("ip:                    Prints the current IPv4 and IPv6 address of this device."));
#ifdef ESP32
		Serial.println(F("scan:                  Scans for WiFi networks in the area and prints the result."));
#endif
		Serial.println(F("help:                  Prints this help text."));
		return true;
	} else {
		return false;
	}
}

void measure() {
#if SENSOR_TYPE == SENSOR_TYPE_DHT
	float temp = dht.readTemperature();
	if (!isnan(temp)) {
		temperature = temp;
	}

	float humid = dht.readHumidity();
	if (!isnan(humid)) {
		humidity = humid;
	}

	if (!isnan(temp) && !isnan(humid)) {
		last_measurement = millis();
	}
#elif SENSOR_TYPE == SENSOR_TYPE_DALLAS
	if (sensors.getDeviceCount() == 0) {
		sensors.begin();
	}
	sensors.requestTemperaturesByIndex(0);
	float temp = sensors.getTempCByIndex(0);
	if (temp != DEVICE_DISCONNECTED_C) {
		temperature = temp;
		last_measurement = millis();
	}
#endif
}

#if ENABLE_WEB_SERVER == 1
std::string getTimeSinceMeasurement() {
	std::ostringstream stream;
	uint64_t now = millis();
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << (now - last_measurement) / 3600000 % 24;
	stream << ':';
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << (now - last_measurement) / 60000 % 60;
	stream << ':';
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << (now - last_measurement) / 1000 % 60;
	stream << '.';
	stream << std::internal << std::setfill('0') << std::setw(3);
	stream << (now - last_measurement) % 1000;
	return stream.str();
}

void registerRequestHandler(const char *uri, WebRequestMethodComposite method,
		HTTPRequestHandler handler) {
	server.on(uri, method,
			[uri, handler](AsyncWebServerRequest *request) {
				prom::http_requests_total[std::pair<String, uint16_t>(uri,
						handler(request))]++;
			});
}

void registerStaticHandler(const char *uri, const char *content_type,
		const char *page) {
	registerRequestHandler(uri, HTTP_GET,
			[content_type, page](AsyncWebServerRequest *request) -> uint16_t {
				request->send_P(200, content_type, page);
				return 200;
			});
}

void registerProcessedStaticHandler(const char *uri, const char *content_type,
		const char *page, const AwsTemplateProcessor processor) {
	registerRequestHandler(uri, HTTP_GET,
			[content_type, page, processor](AsyncWebServerRequest *request) -> uint16_t {
				request->send_P(200, content_type, page, processor);
				return 200;
			});
}

void registerCompressedStaticHandler(const char *uri, const char *content_type,
		const uint8_t *start, const uint8_t *end) {
	registerRequestHandler(uri, HTTP_GET,
			[content_type, start, end](
					AsyncWebServerRequest *request) -> uint16_t {
				const size_t clen = end - start;
				AsyncWebServerResponse *response = NULL;
				if (request->hasHeader("Accept-Encoding")
						&& strstr(
								request->getHeader("Accept-Encoding")->value().c_str(),
								"gzip")) {
					response = request->beginResponse_P(200, content_type,
							start, clen);
					response->addHeader("Content-Encoding", "gzip");
				} else {
					struct uzlib_uncomp decomp;
					uzlib_uncompress_init(&decomp, NULL, 0);
					decomp.source = start;
					decomp.source_limit = end - 4;
					decomp.source_read_cb = NULL;
					uzlib_gzip_parse_header(&decomp);
					response = request->beginResponse(content_type,
							getGzipDecompressedSize(end),
							[clen, decomp](uint8_t *buffer, size_t maxLen,
									size_t index) mutable {
								decomp.dest = buffer;
								decomp.dest_limit = buffer + maxLen;
								uzlib_uncompress(&decomp);
								return decomp.dest - buffer;
							});
				}
				request->send(response);
				return 200;
			});
}
#endif /* ENABLE_WEB_SERVER */

void printTemperature(Print &out, const float temp) {
	out.print("Temperature: ");
	out.print(temperature);
	out.print("°C, ");
	out.print(celsiusToFahrenheit(temperature));
	out.println("°F");
}

float celsiusToFahrenheit(const float celsius) {
	return celsius * 1.8 + 32;
}
