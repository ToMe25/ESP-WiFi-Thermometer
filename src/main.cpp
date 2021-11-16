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

void setup() {
	Serial.begin(115200);
	dht.begin();

	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.printf("WiFi Failed!\n");
		return;
	}

	ipv6_enabled = WiFi.enableIpV6();
	localhost = WiFi.localIP();

	const uint max_wait = 2000;
	const IPv6Address empty = IPv6Address();
	for (int wait = 0; wait < max_wait; wait += 100) {
		if(WiFi.localIPv6().toString() != empty.toString()) {
			break;
		}
		delay(100);
	}

	Serial.println("IP Address: ");
	if (ipv6_enabled) {
		localhost_ipv6 = WiFi.localIPv6();
		Serial.print("IPv6: ");
		Serial.println(localhost_ipv6);
	}
	Serial.print("IPv4: ");
	Serial.println(localhost);

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
		if (ipv6_enabled) {
			Serial.print("IPv6: ");
			Serial.println(localhost_ipv6);
		}
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
