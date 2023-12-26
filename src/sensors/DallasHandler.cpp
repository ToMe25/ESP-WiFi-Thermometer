/*
 * DallasHandler.cpp
 *
 *  Created on: Dec 23, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include "sensors/DallasHandler.h"
#include "fallback_log.h"

namespace sensors {

DallasHandler::DallasHandler(const uint8_t pin, const uint8_t index) :
		SensorHandler(DallasTemperature::millisToWaitForConversion(RESOLUTION)), SENSOR_INDEX(
				index), _wire(pin), _sensors(&_wire) {
}

DallasHandler::~DallasHandler() {
}

bool DallasHandler::begin() {
	_sensors.begin();
	if (_sensors.getDS18Count() == 0) {
		log_e("Couldn't find a DS18 sensor!");
		return false;
	}

	DeviceAddress address;
	if (!_sensors.getAddress(address, SENSOR_INDEX)) {
		log_e("Couldn_t find DS18 sensor with index %u!", SENSOR_INDEX);
		return false;
	}
	_sensors.setResolution(address, RESOLUTION);
	_sensors.setWaitForConversion(false);

	return true;
}

bool DallasHandler::requestMeasurement() {
	if (_last_request == -1 || millis() - _last_request >= MIN_INTERVAL) {
		// Read previous measurements, if they weren't read yet.
		if (_last_request > _last_finished_request) {
			getTemperature();
		}

		_last_request = millis();
		DeviceAddress address;
		if (!_sensors.getAddress(address, SENSOR_INDEX)) {
			log_w("Failed to get address for sensor %u.", SENSOR_INDEX);
			return false;
		}

		if (_sensors.getResolution(address) != RESOLUTION) {
			_sensors.setResolution(address, RESOLUTION);
		}

		if (!_sensors.requestTemperaturesByAddress(address).result) {
			log_w("Failed to read data from DS18 index %u.", SENSOR_INDEX);
			return false;
		}
		return true;
	} else {
		log_i("Attempted to read sensor data before minimum delay.");
		log_d("Min delay: %ums, Time since measurement: %ums",
				(uint32_t) MIN_INTERVAL, (uint32_t) (millis() - _last_request));
		return false;
	}
}

bool DallasHandler::supportsTemperature() const {
	return true;
}

float DallasHandler::getTemperature() {
	if (_last_request > _last_finished_request
			&& millis() - _last_request >= MIN_INTERVAL) {
		DeviceAddress address;
		if (!_sensors.getAddress(address, SENSOR_INDEX)) {
			log_w("Failed to get address for sensor %u.", SENSOR_INDEX);
			_temperature = NAN;
			_last_finished_request = _last_request;
			return _temperature;
		}

		const int32_t temp = _sensors.getTemp(address);
		if (temp == DEVICE_DISCONNECTED_RAW) {
			log_d("Failed to read data from DS18 index %u.", SENSOR_INDEX);
			_temperature = NAN;
		} else if (temp == DEVICE_FAULT_OPEN_RAW) {
			log_d("Failed to read data from DS18 index %u.", SENSOR_INDEX);
			_temperature = NAN;
		} else if (temp == DEVICE_FAULT_SHORTGND_RAW) {
			log_d("DS18 sensor reports short to ground fault.");
			_temperature = NAN;
		} else if (temp == DEVICE_FAULT_SHORTVDD_RAW) {
			log_d("DS18 sensor reports short to vdd fault.");
			_temperature = NAN;
		} else {
			_temperature = DallasTemperature::rawToCelsius(temp);
		}

		_last_finished_request = _last_request;

		if (!std::isnan(_temperature)) {
			_last_valid_temperature = _temperature;
			_last_valid_request = _last_finished_request;
		}
	}

	return _temperature;
}

float DallasHandler::getLastTemperature() {
	if (_last_request > _last_finished_request
			&& millis() - _last_request >= MIN_INTERVAL) {
		getTemperature();
	}
	return _last_valid_temperature;
}

bool DallasHandler::supportsHumidity() const {
	return false;
}

float DallasHandler::getHumidity() {
	return NAN;
}

float DallasHandler::getLastHumidity() {
	return NAN;
}

} /* namespace sensors */
