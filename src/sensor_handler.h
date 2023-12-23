/*
 * sensor_handler.h
 *
 *  Created on: Dec 21, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_SENSOR_HANDLER_H_
#define SRC_SENSOR_HANDLER_H_

#include <string>

namespace sensors {

/**
 * An abstract base class for the classes handling a specific type of sensor.
 *
 * Supports getting the measurements as a float or a string.
 */
class SensorHandler {
protected:
	/**
	 * The minimum time between two measurements in milliseconds.
	 * Depends on the selected resolution.
	 */
	const uint16_t MIN_INTERVAL;

	/**
	 * The system time of the last measurement request in milliseconds.
	 * This request may or may not be finished yet.
	 */
	volatile int64_t _last_request = -1;

	/**
	 * The system time of the last finished measurement request in milliseconds.
	 * This require may or may not have been successful.
	 */
	volatile int64_t _last_finished_request = -1;

	/**
	 * The system time of the last successful measurement request in milliseconds.
	 */
	volatile int64_t _last_valid_request = -1;
public:
	/**
	 * Creates a new SensorHandler and initializes the minimum interval to be used.
	 *
	 * @param min_interval	The minimum interval between two measurements with this sensor.
	 */
	SensorHandler(const uint16_t min_interval);

	/*
	 * Destroys this sensor handler, freeing the underlying sensor connection.
	 */
	virtual ~SensorHandler();

	/**
	 * Initializes the sensor connection.
	 * May or may not request the first measurement.
	 *
	 * @return	Whether initializing the sensor connection was successful.
	 */
	virtual bool begin() = 0;

	/**
	 * Requests a new measurement from the sensor.
	 *
	 * This function is async for all types of sensors that support this, so the measurement may not be available immediately.
	 * It is, however, not guaranteed to be async as some sensors/libraries don't support that.
	 *
	 * This function returns true if requesting the measurements succeeded.
	 * This does not, however, guarantee that the resulting measurements are valid.
	 *
	 * @return	Whether a measurement was successfully requested.
	 */
	virtual bool requestMeasurement() = 0;

	/**
	 * Checks whether this sensor supports measuring the ambient temperature.
	 *
	 * @return	True if the sensor supports temperature measurements.
	 */
	virtual bool supportsTemperature() const = 0;

	/**
	 * Gets the last temperature measurement from the sensor.
	 *
	 * Will return NAN if the last measurement failed.
	 * Will also return NAN if no measurement was taken yet.
	 * And also if the sensor doesn't support temperature measurements.
	 *
	 * @return	The last measured temperature.
	 */
	virtual float getTemperature() = 0;

	/**
	 * Returns the last valid temperature measurement.
	 *
	 * Will be NAN if not a single measurement succeeded yet.
	 * Or the sensor doesn't support temperature measurements.
	 *
	 * @return	The last valid measured temperature.
	 */
	virtual float getLastTemperature() = 0;

	/**
	 * Gets the string representation of the last temperature measurement from the sensor.
	 *
	 * The handler may or may not cache this string.
	 *
	 * Will return "Unknown" if the last measurement failed.
	 * Will also return "Unknown" if no measurement was taken yet.
	 * And also if the sensor doesn't support temperature measurements.
	 *
	 * @return	The last measured temperature.
	 */
	virtual const std::string getTemperatureString();

	/**
	 * Returns the string representation of the last valid temperature measurement.
	 *
	 * The handler may or may not cache this string.
	 *
	 * Will be "Unknown" if not a single measurement succeeded yet.
	 * Or the sensor doesn't support temperature measurements.
	 *
	 * @return	The last measured temperature.
	 */
	virtual const std::string getLastTemperatureString();

	/**
	 * Checks whether this sensor supports measuring the current relative humidity.
	 *
	 * @return	True if the sensor supports relative humidity measurements.
	 */
	virtual bool supportsHumidity() const = 0;

	/**
	 * Gets the last humidity measurement from the sensor.
	 *
	 * Will return NAN if the last measurement failed.
	 * Will also return NAN if no measurement was taken yet.
	 * And also if the sensor doesn't support relative humidity measurements.
	 *
	 * @return	The last measured relative humidity.
	 */
	virtual float getHumidity() = 0;

	/**
	 * Returns the last valid humidity measurement.
	 *
	 * Will be NAN if not a single measurement succeeded yet.
	 * Or the sensor doesn't support relative humidity measurements.
	 *
	 * @return	The last valid measured relative humidity.
	 */
	virtual float getLastHumidity() = 0;

	/**
	 * Gets the string representation of the last humidity measurement from the sensor.
	 *
	 * The handler may or may not cache this string.
	 *
	 * Will return "Unknown" if the last measurement failed.
	 * Will also return "Unknown" if no measurement was taken yet.
	 * And also if the sensor doesn't support relative humidity measurements.
	 *
	 * @return	The last measured relative humidity.
	 */
	virtual const std::string getHumidityString();

	/**
	 * Returns the string representation of the last valid humidity measurement.
	 *
	 * The handler may or may not cache this string.
	 *
	 * Will be "Unknown" if not a single measurement succeeded yet.
	 * Or the sensor doesn't support relative humidity measurements.
	 *
	 * @return	The last measured relative humidity.
	 */
	virtual const std::string getLastHumidityString();

	/**
	 * Returns the time in ms since the last finished measurement was requested.
	 *
	 * This includes both successful and failed measurements.
	 * This does not include measurements that are not finished yet.
	 *
	 * Returns -1 if there was no finished measurement yet.
	 *
	 * @return	The time since the last finished measurement was requested.
	 */
	virtual int64_t getTimeSinceMeasurement();

	/**
	 * Returns the time in ms since the last successfully finished measurement was requested.
	 *
	 * This does not include failed measurements.
	 * This does not include measurements that are not finished yet.
	 *
	 * Returns -1 if there was no successfully finished measurement yet.
	 *
	 * @return	The time since the last finished measurement was requested.
	 */
	virtual int64_t getTimeSinceValidMeasurement();

	/**
	 * Returns the string representation of the time in ms since the last finished measurement was requested.
	 *
	 * The format of this string is "HH:MM:SS.mmm".
	 *
	 * This includes both successful and failed measurements.
	 * This does not include measurements that are not finished yet.
	 *
	 * Returns "Unknown" if there was no finished measurement yet.
	 *
	 * @return	The time since the last finished measurement was requested.
	 */
	virtual const std::string getTimeSinceMeasurementString();

	/**
	 * Returns the string representation of the time in ms since the last successfully finished measurement was requested.
	 *
	 * This does not include failed measurements.
	 * This does not include measurements that are not finished yet.
	 *
	 * Returns "Unknown" if there was no successfully finished measurement yet.
	 *
	 * @return	The time since the last finished measurement was requested.
	 */
	virtual const std::string getTimeSinceValidMeasurementString();

	/**
	 * Gets the minimum time between two measurements with this sensor.
	 * In milliseconds.
	 *
	 * @return	The min time between measurements in ms.
	 */
	virtual uint16_t getMinInterval() const;
};

extern SensorHandler &SENSOR_HANDLER;

}

#endif /* SRC_SENSOR_HANDLER_H_ */
