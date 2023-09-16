/*
 * utils.cpp
 *
 *  Created on: Sep 16, 2023
 *      Author: ToMe25
 */

#include "utils.h"

uint8_t utility::get_msb(const uint32_t number) {
	uint8_t idx = 0;
	while (number >> (idx + 1)) {
		idx++;
	}
	return idx;
}
