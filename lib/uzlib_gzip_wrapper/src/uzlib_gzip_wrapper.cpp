/*
 * uzlib_gzip_wrapper.cpp
 *
 *  Created on: May 26, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include "uzlib_gzip_wrapper.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <cmath>
#include <fallback_log.h>

namespace gzip {

void init() {
	uzlib_init();
}

uzlib_ungzip_wrapper::uzlib_ungzip_wrapper(const uint8_t *cmp_start,
		const uint8_t *cmp_end, int8_t wsize) {
	if (wsize > -8) {
		log_e("Window size out of range.");
		wsize = -8;
	} else if (wsize < -15) {
		log_e("Window size out of range.");
		wsize = -15;
	}

	void *dict = NULL;
	if (cmp_end < cmp_start + 29) {
		log_e("Compressed buffer too small.");
		log_i("A gzip compressed 0 byte file is 29 bytes in size.");
		log_i("The given file was %d bytes.", cmp_end - cmp_start);
	} else {
		dict = malloc(pow(2, -wsize));

		// Read uncompressed size from compressed file.
		dlen = cmp_end[-1];
		dlen = 256 * dlen + cmp_end[-2];
		dlen = 256 * dlen + cmp_end[-3];
		dlen = 256 * dlen + cmp_end[-4];

	}

	decomp = new uzlib_uncomp;
	// Try anyways, since small files can be decompressed without one.
	if (dict == NULL) {
		log_e("Failed to allocate decompression dict.");
	}

	uzlib_uncompress_init(decomp, dict, pow(2, -wsize));
	decomp->source = cmp_start;
	decomp->source_limit = cmp_end - 4 >= cmp_start ? cmp_end - 4 : cmp_start;
	decomp->source_read_cb = NULL;
	uzlib_gzip_parse_header(decomp);
}

uzlib_ungzip_wrapper::uzlib_ungzip_wrapper(int (*callback)(uzlib_uncomp*),
		int8_t wsize) {
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
	decomp->source = NULL;
	decomp->source_limit = NULL;
	decomp->source_read_cb = callback;
	uzlib_gzip_parse_header(decomp);
}

uzlib_ungzip_wrapper::~uzlib_ungzip_wrapper() {
	free(decomp->dict_ring);
	delete decomp;
}

size_t uzlib_ungzip_wrapper::decompress(uint8_t *buf, const size_t buf_size) {
	if (decomp->eof) {
		return 0;
	}

	decomp->dest = buf;
	decomp->dest_limit = buf + buf_size;
	if (dcbuf >= 0) {
		*buf = (uint8_t) dcbuf;
		decomp->dest = buf + 1;
	}

	int res = uzlib_uncompress_chksum(decomp);
	if (res != TINF_OK && res != TINF_DONE) {
		log_e("Decompress failed with error %d.", res);
	}

	const size_t read = decomp->dest - buf;
	index += read;
	if (!decomp->eof) {
		unsigned char dbuf = 0;
		decomp->dest = &dbuf;
		decomp->dest_limit = decomp->dest + 1;
		res = uzlib_uncompress_chksum(decomp);
		if (res == TINF_OK) {
			dcbuf = dbuf;
		} else if (res != TINF_DONE) {
			log_e("Decompress failed with error %d.", res);
		}
	}

	if (decomp->eof) {
		dlen = index;
	}

	return read;
}

int32_t uzlib_ungzip_wrapper::getDecompressedSize() const {
	return dlen;
}

uint32_t uzlib_ungzip_wrapper::getDecompressed() const {
	return index;
}

bool uzlib_ungzip_wrapper::done() const {
	return decomp->eof;
}

} /* namespace gzip */
