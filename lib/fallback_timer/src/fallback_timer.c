/*
 * fallback_timer.h
 *
 * This file contains the implementation for an ESP8266 NonOS fallback definition of the ESP RTOS esp_timer_get_time function.
 *
 * This fallback relies on being called more than once each hour to function.
 *
 *  Created on: Sep 15, 2024
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

// Platform IO seems to ignore the platform specification in a local library.json.
#ifdef ESP8266

#include "fallback_timer.h"
#include <user_interface.h>

/**
 * The current time collected when esp_timer_get_time was last called.
 */
uint32_t last_time = 0;

/**
 * The number of times the current system time overflowed already.
 */
uint32_t time_overflows = 0;

int64_t esp_timer_get_time() {
	const uint32_t now = system_get_time();
	if (now < last_time) {
		time_overflows++;
	}
	last_time = now;
	return (((int64_t) time_overflows) << 32) + now;
}

#endif /* ESP8266 */
