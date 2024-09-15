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
#include <fallback_log.h>
#if SENSOR_TYPE == SENSOR_TYPE_DHT
#include "sensors/DHTHandler.h"
#elif SENSOR_TYPE == SENSOR_TYPE_DALLAS
#include "sensors/DallasHandler.h"
#endif
#ifdef ESP8266
#include <fallback_timer.h>
#endif

namespace sensors {

SensorHandler::SensorHandler(const uint16_t min_interval) :
		MIN_INTERVAL(min_interval) {
}

SensorHandler::~SensorHandler() {
}

const std::string SensorHandler::getTemperatureString() {
	return utils::float_to_string(getTemperature(), 2);
}

const std::string SensorHandler::getLastTemperatureString() {
	return utils::float_to_string(getLastTemperature(), 2);
}

const std::string SensorHandler::getHumidityString() {
	return utils::float_to_string(getHumidity(), 2);
}

const std::string SensorHandler::getLastHumidityString() {
	return utils::float_to_string(getLastHumidity(), 2);
}

int64_t SensorHandler::getTimeSinceMeasurement() {
	const uint64_t now = (uint64_t) esp_timer_get_time() / 1000;
	if (_last_finished_request == -1) {
		return -1;
	} else if (_last_finished_request < 0) {
		log_d("Invalid time of last measurement: %lldms.", _last_finished_request);
		return -1;
	} else if ((uint64_t) _last_finished_request > now) {
		log_d("Invalid time since last measurement: %lldms.", now - _last_finished_request);
		return -1;
	}

	return now - _last_finished_request;
}

int64_t SensorHandler::getTimeSinceValidMeasurement() {
	const uint64_t now = (uint64_t) esp_timer_get_time() / 1000;
	if (_last_valid_request == -1) {
		return -1;
	} else if (_last_valid_request < 0) {
		log_d("Invalid time of last valid measurement: %lldms.", _last_valid_request);
		return -1;
	} else if ((uint64_t) _last_valid_request > now) {
		log_d("Invalid time since last valid measurement: %lldms.", now - _last_valid_request);
		return -1;
	}

	return now - _last_valid_request;
}

const std::string SensorHandler::getTimeSinceMeasurementString() {
	return utils::timespan_to_string(getTimeSinceMeasurement());
}

const std::string SensorHandler::getTimeSinceValidMeasurementString() {
	return utils::timespan_to_string(getTimeSinceValidMeasurement());
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
