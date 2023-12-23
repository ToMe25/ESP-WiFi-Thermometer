/*
 * utils.cpp
 *
 *  Created on: Sep 16, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include "utils.h"
#include <iomanip>
#include <cmath>
#include <sstream>

namespace utility {

uint8_t get_msb(const uint32_t number) {
	uint8_t idx = 0;
	while (number >> (idx + 1)) {
		idx++;
	}
	return idx;
}

float celsiusToFahrenheit(const float celsius) {
	return celsius * 1.8 + 32;
}

std::string float_to_string(const float measurement,
		const uint8_t decimal_digits) {
	if (std::isnan(measurement)) {
		return "Unknown";
	}

	uint8_t precision = decimal_digits + 1;
	if (measurement >= 10) {
		precision++;
	}
	if (measurement >= 100) {
		precision++;
	}

	// TODO check whether snprintf is significantly faster.
	std::ostringstream converter;
	converter << std::setprecision(precision) << measurement;
	return converter.str();
}

std::string timespan_to_string(const int64_t time_ms) {
	if (time_ms < 0) {
		return "Unknown";
	}

	// TODO check whether snprintf is significantly faster.
	std::ostringstream stream;
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << time_ms / 3600000 % 24 << ':';
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << time_ms / 60000 % 60 << ':';
	stream << std::internal << std::setfill('0') << std::setw(2);
	stream << time_ms / 1000 % 60 << '.';
	stream << std::internal << std::setfill('0') << std::setw(3);
	stream << time_ms % 1000;
	return stream.str();
}

} /* namespace utility */
