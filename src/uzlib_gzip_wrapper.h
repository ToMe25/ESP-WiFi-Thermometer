/*
 * uzlib_gzip_wrapper.h
 *
 *  Created on: May 26, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_HTML_UZLIB_GZIP_WRAPPER_H_
#define SRC_HTML_UZLIB_GZIP_WRAPPER_H_

#include <uzlib.h>

/**
 * A wrapper to help with storing the data associated with decompressing a gzip file using uzlib.
 * Can currently only handle the entire compressed file being accessible as a single pointer block.
 */
class uzlib_gzip_wrapper {
private:
	/**
	 * The internal data store used by uzlib.
	 */
	uzlib_uncomp *decomp;

	/**
	 * A pointer to the first byte of compressed data.
	 */
	const uint8_t *const cmp_start;

	/**
	 * The number of already decompressed bytes.
	 */
	size_t index = 0;

public:
	/**
	 * Creates a new gzip wrapper to compress the gzip file in the given memory block.
	 *
	 * @param cmp_start	A pointer to the first byte of the gzip file.
	 * @param cmp_end	A pointer to the first byte after the gzip file.
	 * @param wsize		The window size used for decompression.
	 * 					Has to be at least as much as the window size used for compression.
	 * 					A pow(2, -wsize) byte buffer is allocated for decompression.
	 * 					The range of valid values is from -8 to -15.
	 * 					Values outside of this range will be clamped to this range.
	 */
	uzlib_gzip_wrapper(const uint8_t *cmp_start, const uint8_t *cmp_end,
			int8_t wsize);

	/**
	 * Destroys this gzip wrapper, and removes its internal memory buffer.
	 */
	~uzlib_gzip_wrapper();

	/**
	 * Decompresses the next segment of the gzip file to the given memory buffer.
	 *
	 * @param buf		The memory buffer to write to.
	 * @param buf_size	The max number of bytes to write to the buffer.
	 * @return	The number of bytes written to the buffer.
	 */
	size_t decompress(uint8_t *buf, const size_t buf_size);

	/**
	 * Gets the uncompressed size of the file to be decompressed.
	 *
	 * @return	The uncompressed file size.
	 */
	uint32_t getDecompressedSize() const;
};

#endif /* SRC_HTML_UZLIB_GZIP_WRAPPER_H_ */
