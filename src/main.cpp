/*
 * main.h
 *
 *  Created on: 06.11.2020
 *
 * Copyright (C) 2020-2021 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include <main.h>
#include <iomanip>
#include <regex>
#include <sstream>
#include <Adafruit_Sensor.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

void WiFiEvent(WiFiEvent_t event) {
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
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		WiFi.reconnect();
		break;
	default:
		break;
	}
}

void setupWifi() {
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(1);
	WiFi.onEvent(WiFiEvent);

	if (!WiFi.config(localhost, GATEWAY, INADDR_NONE)) {
		Serial.println("Configuring WiFi failed!");
		return;
	}

	WiFi.begin(WIFI_SSID, WIFI_PASS);
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

uint16_t getMetrics(AsyncWebServerRequest *request) {
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

	request->send(200, "text/plain", metrics.str().c_str());
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
	registerRequestHandler("/metrics", HTTP_GET, getMetrics);

	server.onNotFound(
			[](AsyncWebServerRequest *request) {
				request->send(404, "text/html", NOT_FOUND_HTML);
				Serial.println(request->url().c_str());
				http_requests_total[std::pair<std::string, uint16_t>(
						std::string(request->url().c_str()), 404)]++;
			});

	server.begin();
}

void setupOTA() {
	ArduinoOTA.setHostname(HOSTNAME);
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

void setup() {
	Serial.begin(115200);
	dht.begin();

	setupWifi();
	setupWebServer();
	setupOTA();

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

	used_heap = ESP.getHeapSize() - ESP.getFreeHeap();

	if (loop_iterations == 200) {
		loop_iterations = 0;
	}

	ArduinoOTA.handle();

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
				http_requests_total[std::pair<const char*, uint16_t>(uri,
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
