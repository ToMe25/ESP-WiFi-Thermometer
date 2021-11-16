/*
 * main.h
 *
 *  Created on: 06.11.2020
 *      Author: ToMe25
 */

#include <main.h>
#include <sstream>
#include <regex>
#include <Adafruit_Sensor.h>
#include <WiFi.h>

void getIndex(AsyncWebServerRequest *request) {
	std::string page = INDEX_HTML;
	std::ostringstream converter;
	converter << temperature;
	page = std::regex_replace(page, std::regex("\\$temp"), converter.str());
	converter.clear();
	converter.str("");
	converter << humidity;
	page = std::regex_replace(page, std::regex("\\$humid"), converter.str());
	request->send(200, "text/html", page.c_str());
}

void WiFiEvent(WiFiEvent_t event) {
	switch(event) {
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

void setup() {
	Serial.begin(115200);
	dht.begin();

	setupWifi();

	server.on("/", HTTP_GET, getIndex);
	server.on("/index.html", HTTP_GET, getIndex);

	server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/css", MAIN_CSS);
	});

	server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
		std::ostringstream os;
		os << temperature;
		request->send(200, "text/plain", os.str().c_str());
	});

	server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
		std::ostringstream os;
		os << humidity;
		request->send(200, "text/plain", os.str().c_str());
	});

	server.onNotFound([](AsyncWebServerRequest *request) {
		request->send(404, "text/html", NOT_FOUND_HTML);
	});

	server.begin();
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
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();

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

	if (loop_iterations % 10 == 0) {
		Serial.printf("Temperature: %f°C\n", temperature);
		Serial.printf("Humidity: %f%%\n", humidity);
	}
	loop_iterations++;
	delay(1000);
}
