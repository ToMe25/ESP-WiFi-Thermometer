/*
 * fallback_log.h
 *
 *  Created on: Aug 22, 2023
 *      Author: ToMe25
 */

#ifndef SRC_FALLBACK_LOG_H_
#define SRC_FALLBACK_LOG_H_

#include <cstdio>

/**
 * This file contains simple fallback logging macros, in case there are no other ones available.
 * These aren't that good, but they work well enough.
 *
 * Just like with the esp32 arduino logging macros, the format strings for these macros have to be known at compile time.
 * Otherwise the program will crash at run time.
 */

namespace logging {
/**
 * A constexpr calculating the offset of the last directory separator in a string from the string start.
 *
 * @tparam S	The size of the character array to handle.
 * @param file	The string to calculate the offset for.
 * @param i		The current index in the string to be checked.
 * @return	The calculated start index of the base name.
 */
template<size_t S>
constexpr size_t get_base_name_offset(const char (&file)[S], size_t i = S - 1) {
	return (file[i] == '/' || file[i] == '\\') ? i + 1 : (i > 0 ? get_base_name_offset(file, i - 1) : 0);
}

/**
 * A helper struct holding a constexpr value of a given type.
 * @tparam T	The type of the object to store.
 * @tparam v	The value to store.
 */
template<typename T, T v>
struct const_expr_value {
	static constexpr const T value = v;
};
}

/**
 * CONST_EXPR_VALUE is an utility macro to get force compile time evaluation of a constexpr value.
 */
#define CONST_EXPR_VALUE(exp) logging::const_expr_value<decltype(exp), exp>::value

/**
 * FILE_BASE_NAME is a macro that represents the base name of the current file.
 */
#ifdef __FILE_NAME__
#define FILE_BASE_NAME __FILE_NAME__
#else
#define FILE_BASE_NAME &__FILE__[CONST_EXPR_VALUE(logging::get_base_name_offset(__FILE__))]
#endif

/**
 * GET_BASE_FORMAT_STRING is a utility macro to construct the prefix string for log output.
 */
#define GET_BASE_FORMAT_STRING(letter, ln) "[" #letter "][%s:" EXPAND_MACRO(ln) "] %s(): "

/**
 * EXPAND_MACRO is a utility macro to expand a macro to its string form.
 */
#define EXPAND_MACRO(macro) #macro

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
