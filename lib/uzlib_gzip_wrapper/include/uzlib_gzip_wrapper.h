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
#include <functional>

namespace gzip {

/**
 * Initializes the static variables used by uzlib.
 */
void init();

/**
 * A wrapper to help with storing the data associated with decompressing a gzip file using uzlib.
 * Can currently only handle the entire compressed file being accessible as a single pointer block.
 */
class uzlib_ungzip_wrapper {
private:
	/**
	 * The internal data store used by uzlib.
	 */
	uzlib_uncomp *decomp;

	/**
	 * The number of already decompressed bytes.
	 */
	size_t index = 0;

	/**
	 * The decompressed filesize in bytes module 2^32, if available.
	 * Or -1 if the filesize is not available.
	 */
	int32_t dlen = -1;

	/**
	 * A single decompressed byte, decompressed in advance to work around the incorrect file end detection of uzlib.
	 */
	int16_t dcbuf = -1;

public:
	/**
	 * Creates a new gzip wrapper to decompress the gzip file in the given memory block.
	 *
	 * For this the entire compressed file has to be in a single continuous memory block.
	 *
	 * With this constructor the uncompressed size is available immediately.
	 *
	 * @param cmp_start	A pointer to the first byte of the gzip file.
	 * @param cmp_end	A pointer to the first byte after the gzip file.
	 * @param wsize		The window size used for decompression.
	 * 					Has to be at least as much as the window size used for compression.
	 * 					A pow(2, -wsize) byte buffer is allocated for decompression.
	 * 					The range of valid values is from -8 to -15.
	 * 					Values outside of this range will be clamped to this range.
	 */
	uzlib_ungzip_wrapper(const uint8_t *cmp_start, const uint8_t *cmp_end,
			int8_t wsize);

	/**
	 * Creates a new gzip wrapper to decompress a gzip file from a callback.
	 *
	 * This callback will be called each time `uzlib_uncomp->source` is empty.
	 * This means by default it will be called for every byte to read.
	 *
	 * It is however allowed to modify `uzlib_uncomp->source` and `uzlib_uncomp->source_limit`
	 * in this callback to allow reading a chunk of data at once.
	 * In this case the callback still has to return the **FIRST** read byte.
	 * If `uzlib_uncomp->source` is modified, it must point to the next byte after the returned byte.
	 *
	 * Once all bytes are read the callback must return -1.
	 *
	 * **WARNING:** This callback **MUST NOT** return the last four bytes of a gzip file, since those can't be decompressed.
	 *
	 * With this constructor the uncompressed size is only available once the file is read in its entirety.
	 *
	 * @param callback	The callback to get the compressed data from.
	 * @param wsize		The window size used for decompression.
	 * 					Has to be at least as much as the window size used for compression.
	 * 					A pow(2, -wsize) byte buffer is allocated for decompression.
	 * 					The range of valid values is from -8 to -15.
	 * 					Values outside of this range will be clamped to this range.
	 */
	uzlib_ungzip_wrapper(int (*callback)(uzlib_uncomp*), int8_t wsize);

	/**
	 * Destroys this gzip wrapper, and removes its internal memory buffer.
	 */
	~uzlib_ungzip_wrapper();

	/**
	 * Decompresses the next segment of the gzip file to the given memory buffer.
	 *
	 * Attempts to decompress a single additional byte in advance,
	 * to work around uzlib not detecting the file end when the buffer size exactly matches the uncompressed size.
	 *
	 * @param buf		The memory buffer to write to.
	 * @param buf_size	The max number of bytes to write to the buffer.
	 * @return	The number of bytes written to the buffer.
	 */
	size_t decompress(uint8_t *buf, const size_t buf_size);

	/**
	 * Gets the uncompressed size of the file to be decompressed, if available.
	 *
	 * The uncompressed size is always available once decompression is finished.
	 * If this wrapper was initialized with a memory block, rather than a callback,
	 * the uncompressed size is immediately available.
	 *
	 * @return	The uncompressed file size.
	 */
	int32_t getDecompressedSize() const;

	/**
	 * Gets the number of bytes that were already decompressed.
	 *
	 * @return	The number of already decompressed bytes.
	 */
	uint32_t getDecompressed() const;

	/**
	 * Checks whether the file was decompressed in its entirety.
	 *
	 * @return	Whether the decompression is done.
	 */
	bool done() const;
};

} /* namespace gzip */

#endif /* SRC_HTML_UZLIB_GZIP_WRAPPER_H_ */
