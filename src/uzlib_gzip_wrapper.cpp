/*
 * uzlib_gzip_wrapper.cpp
 *
 *  Created on: May 26, 2023
 *      Author: ToMe25
 */

#include "uzlib_gzip_wrapper.h"
#include <Arduino.h>

uzlib_gzip_wrapper::uzlib_gzip_wrapper(const uint8_t *cmp_start,
		const uint8_t *cmp_end, int8_t wsize) :
		cmp_start(cmp_start) {
	if (wsize > -8) {
		log_e("Window size out of range.");
		wsize = -8;
	} else if (wsize < -15) {
		log_e("Window size out of range.");
		wsize = -15;
	}

	decomp = new uzlib_uncomp;
	void *dict = malloc(pow(2, -wsize));
	// Try anyways, since small files can be decompressed without one.
	if (!dict) {
		log_e("Failed to allocate decompression dict.");
	}
	uzlib_uncompress_init(decomp, dict, pow(2, -wsize));
	decomp->source = cmp_start;
	decomp->source_limit = cmp_end - 4;
	decomp->source_read_cb = NULL;
	uzlib_gzip_parse_header(decomp);
}

uzlib_gzip_wrapper::~uzlib_gzip_wrapper() {
	free(decomp->dict_ring);
	delete decomp;
}

uint32_t uzlib_gzip_wrapper::getDecompressedSize() const {
	size_t dlen = decomp->source_limit[3];
	dlen = 256 * dlen + decomp->source_limit[2];
	dlen = 256 * dlen + decomp->source_limit[1];
	dlen = 256 * dlen + decomp->source_limit[0];
	return dlen;
}

size_t uzlib_gzip_wrapper::decompress(uint8_t *buf, const size_t buf_size) {
	decomp->dest = buf;
	decomp->dest_limit = buf + buf_size;
	int res = uzlib_uncompress(decomp);
	if (res != TINF_OK && res != TINF_DONE) {
		log_e("Decompress failed with error %d.", res);
	}
	return decomp->dest - buf;
}
