/*
 * fallback_timer.h
 *
 * This file contains the header definition for an ESP8266 NonOS fallback definition of the ESP RTOS esp_timer_get_time function.
 *
 * This fallback relies on being called more than once each hour to function.
 *
 *  Created on: Sep 15, 2024
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef LIB_FALLBACK_TIMER_INCLUDE_FALLBACK_TIMER_H_
#define LIB_FALLBACK_TIMER_INCLUDE_FALLBACK_TIMER_H_

// Platform IO seems to ignore the platform specification in a local library.json.
#ifdef ESP8266

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the time in microseconds since the ESP was started.
 *
 * Needs to be called at least once each hour to function.
 *
 * @return The time in microseconds since the ESP first started.
 */
int64_t esp_timer_get_time();


#ifdef __cplusplus
}
#endif

#endif /* ESP8266 */

#endif /* LIB_FALLBACK_TIMER_INCLUDE_FALLBACK_TIMER_H_ */
