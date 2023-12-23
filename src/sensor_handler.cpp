/*
 * sensor_handler.cpp
 *
 *  Created on: Dec 22, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include "config.h"
#include "sensor_handler.h"
#include "utils.h"
#if SENSOR_TYPE == SENSOR_TYPE_DHT
#include "sensors/DHTHandler.h"
#elif SENSOR_TYPE == SENSOR_TYPE_DALLAS
#include "sensors/DallasHandler.h"
#endif

namespace sensors {

SensorHandler::SensorHandler(const uint16_t min_interval) :
		MIN_INTERVAL(min_interval) {
}

SensorHandler::~SensorHandler() {
}

const std::string SensorHandler::getTemperatureString() {
	return utility::float_to_string(getTemperature(), 2);
}

const std::string SensorHandler::getLastTemperatureString() {
	return utility::float_to_string(getLastTemperature(), 2);
}

const std::string SensorHandler::getHumidityString() {
	return utility::float_to_string(getHumidity(), 2);
}

const std::string SensorHandler::getLastHumidityString() {
	return utility::float_to_string(getLastHumidity(), 2);
}

int64_t SensorHandler::getTimeSinceMeasurement() {
	if (_last_finished_request == -1) {
		return -1;
	}

	return millis() - _last_finished_request;
}

int64_t SensorHandler::getTimeSinceValidMeasurement() {
	if (_last_valid_request == -1) {
		return -1;
	}

	return millis() - _last_valid_request;
}

const std::string SensorHandler::getTimeSinceMeasurementString() {
	return utility::timespan_to_string(getTimeSinceMeasurement());
}

const std::string SensorHandler::getTimeSinceValidMeasurementString() {
	return utility::timespan_to_string(getTimeSinceValidMeasurement());
}

uint16_t SensorHandler::getMinInterval() const {
	return MIN_INTERVAL;
}

#if SENSOR_TYPE == SENSOR_TYPE_DHT
DHTHandler dht_handler(SENSOR_PIN, DHT_TYPE);
SensorHandler &SENSOR_HANDLER = dht_handler;
#elif SENSOR_TYPE == SENSOR_TYPE_DALLAS
DallasHandler dallas_handler(SENSOR_PIN, DALLAS_INDEX);
SensorHandler &SENSOR_HANDLER = dallas_handler;
#endif

} /* namespace sensors */
