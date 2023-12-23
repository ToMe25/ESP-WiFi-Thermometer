/*
 * mqtt.cpp
 *
 *  Created on: 04.03.2022
 *      Author: ToMe25
 */

#include "mqtt.h"
#include "main.h"
#include "sensor_handler.h"
#include <iomanip>
#include <sstream>

#if ENABLE_MQTT_PUBLISH == 1
AsyncMqttClient mqtt::mqttClient;
#if ENABLE_DEEP_SLEEP_MODE != 1
uint64_t mqtt::last_publish = 0;
#endif
#endif

void mqtt::setup() {
#if ENABLE_MQTT_PUBLISH == 1
	mqttClient.setServer(MQTT_BROKER_ADDR, MQTT_BROKER_PORT);
	if (strlen(MQTT_TOPIC_NAMESPACE) > 0) {
		mqttClient.setClientId(MQTT_TOPIC_NAMESPACE);
	} else {
		mqttClient.setClientId(HOSTNAME);
	}

#if MQTT_PUBLISH_ANONYMOUS != 1
	mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
#endif
#endif
}

void mqtt::loop() {
#if ENABLE_MQTT_PUBLISH == 1
	publishMeasurements();
#endif
}

void mqtt::connect() {

}

#if ENABLE_MQTT_PUBLISH == 1
void mqtt::publishMeasurements() {
	if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
		mqttClient.connect();
	}

#if ENABLE_DEEP_SLEEP_MODE == 1
	while (!mqttClient.connected()) {
		delay(10);
	}
#endif

	if (mqttClient.connected()) {
#if ENABLE_DEEP_SLEEP_MODE != 1
		uint64_t now = millis();
		if (now - last_publish > MQTT_PUBLISH_INTERVAL * 1000) {
#endif
			String ns;
			if (strlen(MQTT_TOPIC_NAMESPACE) > 0) {
				ns = MQTT_TOPIC_NAMESPACE;
			} else {
				ns = HOSTNAME;
			}

			std::ostringstream converter;
			if (sensors::SENSOR_HANDLER.supportsTemperature()) {
				converter << std::setprecision(3)
						<< sensors::SENSOR_HANDLER.getTemperature();
				if (!mqttClient.publish((ns + "/temperature").c_str(), 0, true,
						converter.str().c_str())) {
					log_w("Failed to publish temperature.");
					return;
				}
			}

			converter.str("");
			converter.clear();

			if (sensors::SENSOR_HANDLER.supportsHumidity()) {
				converter << std::setprecision(3)
						<< sensors::SENSOR_HANDLER.getHumidity();

				if (!mqttClient.publish((ns + "/humidity").c_str(), 0, true,
						converter.str().c_str())) {
					log_w("Failed to publish humidity.");
					return;
				}
			}

#if ENABLE_DEEP_SLEEP_MODE == 1
			std::shared_ptr<bool> connected = std::make_shared<bool>(true);

			mqttClient.onDisconnect(
					[connected](AsyncMqttClientDisconnectReason reason) {
				*connected = false;
			});

			mqttClient.disconnect(false);

			do {
				delay(10);
			} while (*connected);
#else
			last_publish = now;
		}
#endif
	}
}
#endif
