/*
 * DHTHandler.h
 *
 *  Created on: Dec 21, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_SENSORS_DHTHANDLER_H_
#define SRC_SENSORS_DHTHANDLER_H_

#include "sensor_handler.h"
#include <Adafruit_Sensor.h> // Required for DHT library.
#include <DHT.h>

namespace sensors {

/**
 * A handler for a DHT series sensor.
 *
 * The minimum time between measurements depends on the sensor type.
 * One second for DHT 11 and 12, and two for 22 and 21.
 */
class DHTHandler: public SensorHandler {
protected:
	/**
	 * The internal driver used to interact with the sensor.
	 */
	DHT _dht;

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

	/**
	 * The last measured relative humidity.
	 * NAN if the last measurement failed, or no measurement took place yet.
	 */
	volatile float _humidity = NAN;

	/**
	 * The relative humidity from the last successful measurement.
	 * NAN if no measurement succeeded yet.
	 */
	volatile float _last_valid_humidity = NAN;
public:
	/**
	 * Creates a new DHTHandler with the given pin and DHT Type.
	 *
	 * @param pin	The pin the DHT is attached to.
	 * @param type	The type of DHT sensor. Valid values are 11, 12, 21, and 22.
	 */
	DHTHandler(uint8_t pin, uint8_t type);

	/**
	 * Destroys this DHTHandler.
	 */
	virtual ~DHTHandler();

	virtual bool begin() override;
	virtual bool requestMeasurement() override;
	virtual bool supportsTemperature() const override;
	virtual float getTemperature() override;
	virtual float getLastTemperature() override;
	virtual bool supportsHumidity() const override;
	virtual float getHumidity() override;
	virtual float getLastHumidity() override;
};

}

#endif /* SRC_SENSORS_DHTHANDLER_H_ */
