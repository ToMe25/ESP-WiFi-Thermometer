#include <sstream>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define DHTTYPE DHT22
#define DHTPIN 5

AsyncWebServer server(80);

const char *ssid = "WIFI_SSID";
const char *password = "WIFI_PASS";

const char *HTML_START =
		"<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">\n<meta http-equiv=\"refresh\" content=\"5\">\n<title>ESP32-DHT22</title></head>\n<body><center><h1><b>";
const char *HTML_END = "</b></h1></center></body></html>";

DHT dht(DHTPIN, DHTTYPE);

IPAddress localhost;
IPv6Address localhost_ipv6;

float temperature;
float humidity;

std::string command;

uint loop_iterations = 0;

bool ipv6_enabled;

void notFound(AsyncWebServerRequest *request) {
	request->send(404, "text/plain", "Not found");
}

void setup() {
	Serial.begin(115200);
	dht.begin();

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
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

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		std::ostringstream os;
		os << HTML_START;
		os << "Temperature: ";
		os << temperature;
		os << "°C<br>\nHumidity: ";
		os << humidity;
		os << "%";
		os << HTML_END;
		request->send(200, "text/html", os.str().c_str());
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

	server.onNotFound(notFound);

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
