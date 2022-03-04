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
#include <PubSubClient.h>
#endif

/**
 * This header and the source file with the same name contain the MQTT integration.
 */
namespace mqtt {
#if ENABLE_MQTT_PUBLISH == 1
extern WiFiClient wifiClient;
extern PubSubClient mqttClient;
#if ENABLE_DEEP_SLEEP_MODE != 1
extern uint64_t last_publish;
#endif
#endif

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
