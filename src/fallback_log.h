/*
 * fallback_log.h
 *
 * This file contains simple fallback logging macros, in case there are no other ones available.
 * These aren't that good, but they work well enough.
 *
 * Just like with the esp32 arduino logging macros, the format strings for these macros have to be known at compile time.
 * Otherwise the program will crash at run time.
 *
 *  Created on: Aug 22, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_FALLBACK_LOG_H_
#define SRC_FALLBACK_LOG_H_

#include <cstdio>
#include "utils.h"

/**
 * GET_BASE_FORMAT_STRING is a utility macro to construct the prefix string for log output.
 */
#define GET_BASE_FORMAT_STRING(letter, ln) "[" #letter "][%s:" EXPAND_MACRO(ln) "] %s(): "

/**
 * An internal utility macro adding some debug info, and giving the strings to printf.
 *
 * @param letter	The letter representing the log level of the message.
 * @param format	The format string to print. Has to be known at compile time.
 */
#define LOG_PRINTF(letter, format, ...) printf(GET_BASE_FORMAT_STRING(letter, __LINE__) format "\r\n", FILE_BASE_NAME, __FUNCTION__, ##__VA_ARGS__);

/**
 * Below are fallback definitions for the arduino logging macros.
 */

#ifndef log_v
#if CORE_DEBUG_LEVEL >= 5
#define log_v(format, ...) LOG_PRINTF(V, format, ##__VA_ARGS__)
#else
#define log_v(format, ...)
#endif
#endif

#ifndef log_d
#if CORE_DEBUG_LEVEL >= 4
#define log_d(format, ...) LOG_PRINTF(D, format, ##__VA_ARGS__)
#else
#define log_d(format, ...)
#endif
#endif

#ifndef log_i
#if CORE_DEBUG_LEVEL >= 3
#define log_i(format, ...) LOG_PRINTF(I, format, ##__VA_ARGS__)
#else
#define log_i(format, ...)
#endif
#endif

#ifndef log_w
#if CORE_DEBUG_LEVEL >= 2
#define log_w(format, ...) LOG_PRINTF(W, format, ##__VA_ARGS__)
#else
#define log_w(format, ...)
#endif
#endif

#ifndef log_e
#if CORE_DEBUG_LEVEL >= 1
#define log_e(format, ...) LOG_PRINTF(E, format, ##__VA_ARGS__)
#else
#define log_e(format, ...)
#endif
#endif

#endif /* SRC_FALLBACK_LOG_H_ */
