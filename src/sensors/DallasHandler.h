/*
 * DallasHandler.h
 *
 *  Created on: Dec 23, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_SENSORS_DALLASHANDLER_H_
#define SRC_SENSORS_DALLASHANDLER_H_

#include "sensor_handler.h"
#include "DallasTemperature.h"

namespace sensors {

/**
 * A handler for a Dallas temperature sensor.
 *
 * The minimum time between two measurements depends on the selected resolution.
 */
class DallasHandler: public SensorHandler {
protected:
	/**
	 * The index of the sensor to get the measurements from.
	 */
	const uint8_t SENSOR_INDEX;

	/**
	 * The measurement resolution for the sensor.
	 * Range: 9-12 bit.
	 * Note: The sensor default is 12 bit.
	 */
	static const uint8_t RESOLUTION = 12;

	/**
	 * The internal OneWire instance to use to communicate with the sensor.
	 */
	OneWire _wire;

	/**
	 * The internal DallasTemperature instance to use to get sensor measurements.
	 */
	DallasTemperature _sensors;

	/**
	 * The last measured temperature.
	 * NAN if the last measurement failed, or no measurement took place yet.
	 */
	volatile float _temperature = NAN;

	/**
	 * The temperature from the last successful measurement.
	 * NAN if no measurement succeeded yet.
	 */
	volatile float _last_valid_temperature = NAN;
public:
	/**
	 * Creates a new DallasHandler with the given sensor pin and sensor index.
	 *
	 * @param pin	The pin to which the sensor is connected.
	 * @param index	The index of the sensor to use.
	 */
	DallasHandler(const uint8_t pin, const uint8_t index);

	/**
	 * Destroys this DallasHandler.
	 */
	virtual ~DallasHandler();

	virtual bool begin() override;
	virtual bool requestMeasurement() override;
	virtual bool supportsTemperature() const override;
	virtual float getTemperature() override;
	virtual float getLastTemperature() override;
	virtual bool supportsHumidity() const override;
	virtual float getHumidity() override;
	virtual float getLastHumidity() override;
};

} /* namespace sensors */

#endif /* SRC_SENSORS_DALLASHANDLER_H_ */
