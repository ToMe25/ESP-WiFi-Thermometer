/*
 * DHTHandler.cpp
 *
 *  Created on: Dec 21, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include "sensors/DHTHandler.h"
#include "fallback_log.h"

namespace sensors {

DHTHandler::DHTHandler(uint8_t pin, uint8_t type) :
		SensorHandler((type == 11 || type == 12) ? 1000 : 2000), _dht(
				pin, type) {
}

DHTHandler::~DHTHandler() {
}

bool DHTHandler::begin() {
	_dht.begin();
	// TODO check whether the sensor actually exists.
	return true;
}

bool DHTHandler::requestMeasurement() {
	if (_last_request == -1 || millis() - _last_request >= MIN_INTERVAL) {
		_last_request = millis();
		if (!_dht.read(false)) {
			_temperature = _humidity = NAN;
			log_w("Failed to read data from dht.");
			return false;
		}

		_temperature = _dht.readTemperature(false);
		_humidity = _dht.readHumidity(false);
		_last_finished_request = _last_request;

		// Measurements are considered to be either entirely valid, or entirely invalid.
		if (!std::isnan(_temperature) && !std::isnan(_humidity)) {
			_last_valid_temperature = _temperature;
			_last_valid_humidity = _humidity;
			_last_valid_request = _last_finished_request;
		} else {
			log_i("Read partially invalid data from dht.");
		}

		return true;
	} else {
		log_i("Attempted to read sensor data before minimum delay.");
		log_d("Min delay: %ums, Time since measurement: %ums",
				(uint32_t) MIN_INTERVAL, (uint32_t) (millis() - _last_request));
		return false;
	}
}

bool DHTHandler::supportsTemperature() const {
	return true;
}

float DHTHandler::getTemperature() {
	return _temperature;
}

float DHTHandler::getLastTemperature() {
	return _last_valid_temperature;
}

bool DHTHandler::supportsHumidity() const {
	return true;
}

float DHTHandler::getHumidity() {
	return _humidity;
}

float DHTHandler::getLastHumidity() {
	return _last_valid_humidity;
}

} /* namespace sensors */
