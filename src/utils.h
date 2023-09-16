/*
 * utils.h
 *
 * This file contains some general utilities that didn't fit any other file.
 *
 *  Created on: Sep 16, 2023
 *      Author: ToMe25
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <cstdint>
#include <cstdlib>

namespace utility {
/**
 * A method to get the index of the Most Significant Bit set in a number.
 * Necessary because the std log2 function doesn't exist on arm hosts.
 *
 * @param number	The number to get the MSB for.
 * @return	The index of the Most Significant Bit set in the number.
 */
uint8_t get_msb(const uint32_t number);

/**
 * A constexpr calculating the offset of the last directory separator in a string from the string start.
 *
 * @tparam S	The size of the character array to handle.
 * @param file	The string to calculate the offset for.
 * @param i		The current index in the string to be checked.
 * @return	The calculated start index of the base name.
 */
template<size_t S>
constexpr size_t get_base_name_offset(const char (&file)[S],
		const size_t i = S - 1) {
	return (file[i] == '/' || file[i] == '\\') ?
			i + 1 : (i > 0 ? get_base_name_offset(file, i - 1) : 0);
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

/**
 * Determines the length of the given string as a constant expression.
 *
 * @param str	The string to get the length of.
 * @return	The number of characters in the given string.
 */
constexpr size_t strlen(const char *str) {
	return (*str == 0) ? 0 : strlen(str + 1) + 1;
}
}

/**
 * CONST_EXPR_VALUE is an utility macro to get force compile time evaluation of a constexpr value.
 */
#define CONST_EXPR_VALUE(exp) utility::const_expr_value<decltype(exp), exp>::value

/**
 * FILE_BASE_NAME is a macro that represents the base name of the current file.
 */
#ifdef __FILE_NAME__
#define FILE_BASE_NAME __FILE_NAME__
#else
#define FILE_BASE_NAME &__FILE__[CONST_EXPR_VALUE(utility::get_base_name_offset(__FILE__))]
#endif

/**
 * EXPAND_MACRO is a utility macro to expand a macro to its string form.
 */
#define EXPAND_MACRO(macro) #macro

#endif /* SRC_UTILS_H_ */
