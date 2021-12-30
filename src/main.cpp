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
#include <iomanip>
#include <regex>
#include <sstream>
#include <Adafruit_Sensor.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

IPAddress localhost = STATIC_IP;
IPv6Address localhost_ipv6;
AsyncWebServer server(WEB_SERVER_PORT);
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
uint64_t start = 0;

void setup() {
	start = millis();
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

#if ENABLE_DEEP_SLEEP_MODE == 1
	measure();

	Serial.print("Temperature: ");
	Serial.print(temperature);
	Serial.println("°C");
	Serial.print("Humidity: ");
	Serial.print(humidity);
	Serial.println('%');

	if (WiFi.waitForConnectResult() == WL_CONNECTED) {
#if ENABLE_PROMETHEUS_PUSH == 1
		prom::pushMetrics();
#endif
	} else {
		Serial.println("Failed to connect to WiFi!");
	}

	WiFi.disconnect(1, 1);
	esp_sleep_enable_timer_wakeup(DEEP_SLEEP_MODE_MEASUREMENT_INTERVAL * 1000000 - (micros() - start * 1000));
	esp_deep_sleep_start();
#endif

	prom::setup();
}

void onWiFiEvent(WiFiEventId_t id, WiFiEventInfo_t info) {
	switch (id) {
	case SYSTEM_EVENT_STA_START:
		WiFi.setHostname(HOSTNAME);
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		WiFi.enableIpV6();
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
		Serial.print(millis() - start);
		Serial.println("ms after start.");
		Serial.print("STA IP: ");
		Serial.println(localhost = WiFi.localIP());
		prom::connect();
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

void setupWiFi() {
	WiFi.mode(WIFI_STA);
#if ENABLE_DEEP_SLEEP_MODE != 1
	WiFi.disconnect(1);
#endif
	WiFi.onEvent(onWiFiEvent);

	if (!WiFi.config(localhost, GATEWAY, SUBNET)) {
		Serial.println("Configuring WiFi failed!");
		return;
	}

	WiFi.begin(WIFI_SSID, WIFI_PASS);
}

#if ENABLE_ARDUINO_OTA == 1
void setupOTA() {
	ArduinoOTA.setHostname(HOSTNAME);
	ArduinoOTA.setPort(ARDUINO_OTA_PORT);
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
	registerRequestHandler("/", HTTP_GET, getIndex);
	registerRequestHandler("/index.html", HTTP_GET, getIndex);

	registerStaticHandler("/main.css", "text/css", MAIN_CSS);
	registerStaticHandler("/index.js", "text/javascript", INDEX_JS);

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

	registerRequestHandler("/favicon.ico", HTTP_GET,
			[](AsyncWebServerRequest *request) -> uint16_t {
				AsyncProgmemResponse response(200, "image/x-icon",
						FAVICON_ICO_START, FAVICON_ICO_END - FAVICON_ICO_START);
				request->send(&response);
				return 200;
			});

	server.onNotFound(
			[](AsyncWebServerRequest *request) {
				request->send(404, "text/html", NOT_FOUND_HTML);
				Serial.print("A client tried to access the not existing file \"");
				Serial.print(request->url().c_str());
				Serial.println("\".");
				prom::http_requests_total[std::pair<std::string, uint16_t>(
						std::string(request->url().c_str()), 404)]++;
			});

	server.begin();

#if ENABLE_ARDUINO_OTA != 1
	MDNS.begin(HOSTNAME);
#endif

	MDNS.addService("http", "tcp", 80);
}

uint16_t getIndex(AsyncWebServerRequest *request) {
	std::string page = INDEX_HTML;
	std::ostringstream converter;
	converter << std::setprecision(3) << temperature;
	page = std::regex_replace(page, std::regex("\\$temp"), converter.str());
	converter.clear();
	converter.str("");
	converter << std::setprecision(3) << humidity;
	page = std::regex_replace(page, std::regex("\\$humid"), converter.str());
	page = std::regex_replace(page, std::regex("\\$time"), getTimeSinceMeasurement());
	request->send(200, "text/html", page.c_str());
	return 200;
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
	request->send(200, "application/json", json.str().c_str());
	return 200;
}
#endif /* ENABLE_WEB_SERVER */

bool handle_serial_input(std::string input) {
	if (input == "temperature" || input == "temp") {
		Serial.println();
		Serial.print("Temperature: ");
		Serial.print(temperature);
		Serial.print("°C, ");
		Serial.print(celsiusToFahrenheit(temperature));
		Serial.println("°F");
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
		Serial.print("IPv6: ");
		Serial.println(localhost_ipv6);
		Serial.print("IPv4: ");
		Serial.println(localhost);
		return true;
	} else if (input == "scan") {
		Serial.println();
		Serial.println("Starting WiFi scan...");
		WiFi.scanNetworks(true, true);
		return true;
	} else if (input == "help") {
		Serial.println();
		Serial.println("ESP-WiFi-Thermometer help:");
		Serial.println("temperature (or temp): Prints the last measured temperature in °C and °F.");
		Serial.println("humidity:              Prints the relative humidity in %.");
		Serial.println("ip:                    Prints the current IPv4 and IPv6 address of this device.");
		Serial.println("scan:                  Scans for WiFi networks in the area and prints the result.");
		Serial.println("help:                  Prints this help text.");
		return true;
	} else {
		return false;
	}
}

void loop() {
	uint64_t start = millis();

	if (loop_iterations % 4 == 0) {
		measure();

		if (loop_iterations % 20 == 0 && millis() - last_measurement < 10000) {
			Serial.print("Temperature: ");
			Serial.print(temperature);
			Serial.print("°C, ");
			Serial.print(celsiusToFahrenheit(temperature));
			Serial.println("°F");
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

	ArduinoOTA.handle();

	prom::loop();

	loop_iterations++;
	uint64_t end = millis();
	delay(max(0, 500 - int16_t(end - start)));
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
	sensors.requestTemperatures();
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
				prom::http_requests_total[std::pair<const char*, uint16_t>(uri,
						handler(request))]++;
			});
}

void registerStaticHandler(const char *uri, const char *content_type,
		const char *page) {
	registerRequestHandler(uri, HTTP_GET,
			[content_type, page](AsyncWebServerRequest *request) -> uint16_t {
				request->send(200, content_type, page);
				return 200;
			});
}
#endif /* ENABLE_WEB_SERVER */

float celsiusToFahrenheit(float celsius) {
	return celsius * 1.8 + 32;
}
