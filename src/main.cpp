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
DHT dht(DHT_PIN, DHT_TYPE);
float temperature = 0;
float humidity = 0;
std::chrono::time_point<std::chrono::system_clock> last_measurement;
std::string command;
uint8_t loop_iterations = 0;

void setup() {
	Serial.begin(115200);
	dht.begin();

	setupWiFi();
#if ENABLE_ARDUINO_OTA == 1
	setupOTA();
#endif
	setupWebServer();

	prom::setup();
}

void onWiFiEvent(WiFiEvent_t event) {
	switch (event) {
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
		Serial.print("STA IP: ");
		Serial.println(localhost = WiFi.localIP());
		prom::connect();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		WiFi.reconnect();
		break;
	default:
		break;
	}
}

void setupWiFi() {
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(1);
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

	MDNS.addService("http", "tcp", 80);
}

bool handle_serial_input(std::string input) {
	if (input == "ip") {
		Serial.println("IP Address: ");
		Serial.print("IPv6: ");
		Serial.println(localhost_ipv6);
		Serial.print("IPv4: ");
		Serial.println(localhost);
		return true;
	} else if (input == "temperature" || input == "temp") {
		Serial.printf("Temperature: %f°C, %f°F\n", temperature,
				dht.convertCtoF(temperature));
		return true;
	} else if (input == "humidity") {
		Serial.printf("Humidity: %f%%\n", humidity);
		return true;
	} else {
		return false;
	}
}

void loop() {
	uint64_t start = millis();

	if (loop_iterations % 4 == 0) {
		float temp = dht.readTemperature();
		if (!isnan(temp)) {
			temperature = temp;
		}

		float humid = dht.readHumidity();
		if (!isnan(humid)) {
			humidity = humid;
		}

		if (!isnan(temp) && !isnan(humid)) {
			last_measurement = std::chrono::system_clock::now();
		}

		if (loop_iterations % 20 == 0) {
			Serial.printf("Temperature: %f°C\n", temperature);
			Serial.printf("Humidity: %f%%\n", humidity);
		}
	}

	uint available = Serial.available();
	if (available > 0) {
		char *input = new char[available];
		Serial.readBytes(input, available);
		command += std::string(input).substr(0, available);

		size_t pos = 0;
		std::string cmd;
		while ((pos = command.find("\n")) != std::string::npos) {
			cmd = command.substr(0, pos - 1);
			command.erase(0, pos + 1);
			if (!handle_serial_input(cmd)) {
				Serial.print("Unknown Command: ");
				Serial.println(cmd.c_str());
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
	delay(max(0, 500 - int16_t(start - end)));
}

std::string getTimeSinceMeasurement() {
	using namespace std::chrono;
	std::ostringstream stream;
	time_point<system_clock> now = system_clock::now();
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << duration_cast<hours>(now - last_measurement).count() % 24;
	stream << ':';
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << duration_cast<minutes>(now - last_measurement).count() % 60;
	stream << ':';
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << duration_cast<seconds>(now - last_measurement).count() % 60;
	stream << '.';
	stream << std::internal << std::setfill('0') << std::setw(3);
	stream << duration_cast<milliseconds>(now - last_measurement).count() % 1000;
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
