/*
 * mqtt.h
 *
 *  Created on: 04.03.2022
 *      Author: ToMe25
 */

#ifndef SRC_MQTT_H_
#define SRC_MQTT_H_

#include "config.h"
#if ENABLE_MQTT_PUBLISH == 1
#include <AsyncMqttClient.h>
#endif

/**
 * This header and the source file with the same name contain the MQTT integration.
 */
namespace mqtt {
#if ENABLE_MQTT_PUBLISH == 1
extern AsyncMqttClient mqttClient;
#if ENABLE_DEEP_SLEEP_MODE != 1
extern uint64_t last_publish;
#endif
#endif

// TODO make optional, somehow
// Includes the content of the file "mqttuser.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char MQTT_USER[] asm("_binary_mqttuser_txt_start");
// Includes the content of the file "mqttpass.txt" in the project root.
// Make sure this file doesn't end with an empty line.
extern const char MQTT_PASS[] asm("_binary_mqttpass_txt_start");

/**
 * Initializes the MQTT integration.
 */
void setup();

/**
 * A method that does everything that should be done every loop iteration.
 */
void loop();

/**
 * A handler for things that should happen when a new WiFi connection is established.
 */
void connect();

#if ENABLE_MQTT_PUBLISH == 1
/**
 * The method that handles publishing the measurements to the MQTT broker.
 */
void publishMeasurements();
#endif
}

#endif /* SRC_MQTT_H_ */
