/*
 * decompress.cpp
 *
 *  Created on: Jan 5, 2024
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#include <unity.h>
#include <uzlib_gzip_wrapper.h>
#include <utils.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>

/**
 * Use a constant seed, to get reproducible results.
 * Used to generate the random data for the files for compression tests.
 */
const std::mt19937::result_type RANDOM_SEED = 1685018244;

/**
 * The characters to be used as part of the generated random data.
 */
constexpr char RANDOM_CHARS[] =
		"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/**
 * The number of random characters to use.
 */
constexpr size_t RANDOM_CHARS_LEN = utils::strlen(RANDOM_CHARS);

/**
 * The command to be used to compress a file, without the filename component.
 * A window size of -10 means a 1024 byte buffer, while -15(the max) means a 32768 byte buffer.
 */
const char BASE_COMMAND[] =
		"python3 -m shared.gzip_compressing_stream --window-size -10 ";

/**
 * The path to the uncompressed random data file.
 * Initialized in setUp and deleted in tearDown.
 */
char *uncompressed_path;

/**
 * The path for the compressed gzip file.
 * Initialized in setUp and deleted in tearDown.
 */
char *compressed_path;

/**
 * The command to execute to compress the uncompressed file.
 * Initialized in setUp and deleted in tearDown.
 */
char *compress_command;

/**
 * The integer distribution for the random data.
 * Initialized in setUp and destroyed in tearDown.
 */
std::uniform_int_distribution<uint8_t> *distribution;

/**
 * A pointer to the random number generator to be used.
 * Initialized in setUp and destroyed in tearDown.
 */
std::mt19937 *rng;

/**
 * A pointer to a file input stream for the compressed file.
 * To be populated by the test. Will be deleted in tearDown.
 */
std::ifstream *compressed_in = NULL;

/**
 * The length of the compressed file.
 * Has to be determined and set by the test.
 */
size_t compressed_length = 0;

/**
 * Generates a new path for the uncompressed file, and initializes the compressed path to match it.
 * Also initializes the random number generator.
 */
void setUp() {
	uncompressed_path = new char[L_tmpnam];
	uncompressed_path = tmpnam(uncompressed_path);
	if (uncompressed_path) {
		const size_t u_len = std::strlen(uncompressed_path);
		compressed_path = new char[u_len + 4]; // 3 for .gz plus NUL byte.
		strncpy(compressed_path, uncompressed_path, u_len + 1);
		strncpy(compressed_path + u_len, ".gz", 4);

		const size_t bc_len = std::strlen(BASE_COMMAND);
		compress_command = new char[bc_len + u_len + 1];
		strncpy(compress_command, BASE_COMMAND, bc_len + 1);
		strncpy(compress_command + bc_len, uncompressed_path, u_len + 1);
	}

	rng = new std::mt19937(RANDOM_SEED);
	distribution = new std::uniform_int_distribution<uint8_t>(0,
			RANDOM_CHARS_LEN - 1);
}

/**
 * Deletes the compressed and uncompressed file, and destroys the random number generator.
 */
void tearDown() {
	remove(uncompressed_path);
	delete[] uncompressed_path;
	uncompressed_path = NULL;
	remove(compressed_path);
	delete[] compressed_path;
	compressed_path = NULL;
	delete[] compress_command;
	compress_command = NULL;
	delete rng;
	rng = NULL;
	delete distribution;
	distribution = NULL;
	delete compressed_in;
	compressed_in = NULL;
}

/**
 * Checks that the temporary path was successfully generated, and a python interpreter is available.
 */
void check_fixtures() {
	TEST_ASSERT_NOT_NULL_MESSAGE(uncompressed_path,
			"Failed to generate temporary file path.");
	TEST_ASSERT_NOT_NULL_MESSAGE(compressed_path,
			"Failed to generate temporary file path.");
	TEST_ASSERT_NOT_NULL_MESSAGE(compress_command,
			"Failed to assemble command string.");

	// Assume that a successful command returns 0.
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, std::system("python3 -V"),
			"Failed to find python 3 interpreter.");
}

/**
 * Writes the given number of random bytes to the given file output stream.
 *
 * This function generates only bytes matching this regex: `[0-9a-zA-Z]`.
 *
 * @param output	The file output stream to write to.
 * @param bytes		The number of bytes to write.
 */
void write_random_data(std::ofstream &output, const uint32_t bytes) {
	for (size_t i = 0; i < bytes; i++) {
		output << RANDOM_CHARS[(*distribution)(*rng)];
	}
}

/**
 * Creates an uncompressed file with random data of the given size, and compresses it.
 *
 * @param size	The size of the uncompressed file, in bytes.
 */
void prepare_compressed_file(const size_t size) {
	std::ofstream uncompressed_out;
	uncompressed_out.open(uncompressed_path);
	TEST_ASSERT_TRUE_MESSAGE(uncompressed_out.is_open(),
			"Failed to open uncompressed file.");
	write_random_data(uncompressed_out, size);
	uncompressed_out.close();
	TEST_ASSERT_FALSE_MESSAGE(uncompressed_out.fail(),
			"Writing uncompressed file failed.");

	// Assume that a successful command returns 0.
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, std::system(compress_command),
			"GZIP compression command failed.");
}

/**
 * Test decompressing a file that is small enough to fit into the decompression buffer.
 */
void test_decompress_small() {
	// The size of the uncompressed file used for testing.
	const size_t FILE_SIZE = 512;
	// The size of the block of data to be compared when checking the file.
	const size_t TEST_COMP_SIZE = 25;

	check_fixtures();
	prepare_compressed_file(FILE_SIZE);

	compressed_in = new std::ifstream();
	compressed_in->open(compressed_path, std::ios::in | std::ios::binary);
	TEST_ASSERT_TRUE_MESSAGE(compressed_in->is_open(),
			"Failed to open compressed file.");
	char *compressed = new char[FILE_SIZE];
	compressed_length = compressed_in->read(compressed, FILE_SIZE).gcount();
	compressed_in->close();

	gzip::uzlib_ungzip_wrapper unzip((uint8_t*) compressed,
			(uint8_t*) compressed + compressed_length, -10);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(FILE_SIZE, unzip.getDecompressedSize(),
			"The initial decompressed size didn't match expectations.");
	std::ifstream uncompressed_in;
	uncompressed_in.open(uncompressed_path);
	TEST_ASSERT_TRUE_MESSAGE(uncompressed_in.is_open(),
			"Failed to open uncompressed file.");
	char *uncompressed = new char[FILE_SIZE];
	uncompressed_in.read(uncompressed, FILE_SIZE);
	uncompressed_in.close();

	char *decompressed = new char[FILE_SIZE];
	TEST_ASSERT_EQUAL_UINT_MESSAGE(FILE_SIZE,
			unzip.decompress((uint8_t* ) decompressed, FILE_SIZE),
			"The number of decompressed bytes didn't match the decompressed size.");
	TEST_ASSERT_TRUE_MESSAGE(unzip.done(),
			"The decompression wasn't considered done after decompressing everything.");
	TEST_ASSERT_EQUAL_INT32_MESSAGE(FILE_SIZE, unzip.getDecompressedSize(),
			"The final decompressed size didn't match expectations.");

	// Compare content in blocks of TEST_COMP_SIZE characters, for better error readability.
	const size_t FILE_SIZE_LEN = log10(FILE_SIZE) + 1;
	for (size_t start = 0; start + TEST_COMP_SIZE < FILE_SIZE; start +=
			TEST_COMP_SIZE) {
		const size_t comp_len = std::min(TEST_COMP_SIZE, FILE_SIZE - start);
		char message[58 + FILE_SIZE_LEN * 2];
		snprintf(message, 58 + FILE_SIZE_LEN * 2,
				"Decompressed file bytes %lu-%lu don't match uncompressed file.",
				start, start + comp_len);
		TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(uncompressed + start,
				decompressed + start, comp_len, message);
	}

	delete[] compressed;
	delete[] uncompressed;
	delete[] decompressed;
}

/**
 * Test decompressing a file that too large to fit into the decompressing buffer, from memory to memory.
 */
void test_decompress_large() {
	// The size of the uncompressed file used for testing.
	const size_t FILE_SIZE = 131072;
	// The size of the block of data to be compared when checking the file.
	const size_t TEST_COMP_SIZE = 25;

	check_fixtures();
	prepare_compressed_file(FILE_SIZE);

	compressed_in = new std::ifstream();
	compressed_in->open(compressed_path, std::ios::in | std::ios::binary);
	TEST_ASSERT_TRUE_MESSAGE(compressed_in->is_open(),
			"Failed to open compressed file.");
	char *compressed = new char[FILE_SIZE];
	compressed_length = compressed_in->read(compressed, FILE_SIZE).gcount();
	compressed_in->close();

	gzip::uzlib_ungzip_wrapper unzip((uint8_t*) compressed,
			(uint8_t*) compressed + compressed_length, -10);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(FILE_SIZE, unzip.getDecompressedSize(),
			"The initial decompressed size didn't match expectations.");
	std::ifstream uncompressed_in;
	uncompressed_in.open(uncompressed_path);
	TEST_ASSERT_TRUE_MESSAGE(uncompressed_in.is_open(),
			"Failed to open uncompressed file.");
	char *uncompressed = new char[FILE_SIZE];
	uncompressed_in.read(uncompressed, FILE_SIZE);
	uncompressed_in.close();

	char *decompressed = new char[FILE_SIZE];
	TEST_ASSERT_EQUAL_UINT_MESSAGE(FILE_SIZE,
			unzip.decompress((uint8_t* ) decompressed, FILE_SIZE),
			"The number of decompressed bytes didn't match the decompressed size.");
	TEST_ASSERT_TRUE_MESSAGE(unzip.done(),
			"The decompression wasn't considered done after decompressing everything.");
	TEST_ASSERT_EQUAL_INT32_MESSAGE(FILE_SIZE, unzip.getDecompressedSize(),
			"The final decompressed size didn't match expectations.");

	// Compare content in blocks of TEST_COMP_SIZE characters, for better error readability.
	const size_t FILE_SIZE_LEN = log10(FILE_SIZE) + 1;
	for (size_t start = 0; start + TEST_COMP_SIZE < FILE_SIZE; start +=
			TEST_COMP_SIZE) {
		const size_t comp_len = std::min(TEST_COMP_SIZE, FILE_SIZE - start);
		char message[58 + FILE_SIZE_LEN * 2];
		snprintf(message, 58 + FILE_SIZE_LEN * 2,
				"Decompressed file bytes %lu-%lu don't match uncompressed file.",
				start, start + comp_len);
		TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(uncompressed + start,
				decompressed + start, comp_len, message);
	}

	delete[] compressed;
	delete[] uncompressed;
	delete[] decompressed;
}

/**
 * The uzlib read callback for test_decompress_streaming.
 *
 * @param uncomp	The uzlib_uncomp instance to read data for.
 * @return	The first read byte, or -1 if the file was read in its entirety already.
 */
int read_compressed_callback(uzlib_uncomp *uncomp) {
	// The size of the internal read buffer.
	const size_t RDBUFSIZ = 1024;

	if (compressed_in->eof()
			|| (size_t) compressed_in->tellg() >= compressed_length - 4) {
		if (uncomp->source != NULL) {
			uncomp->source -= (compressed_length - 4) % RDBUFSIZ - 1;
			delete[] (uncomp->source - 1);
			uncomp->source = NULL;
			uncomp->source_limit = NULL;
		}
		return -1;
	}

	if (uncomp->source == NULL) {
		uncomp->source = new unsigned char[RDBUFSIZ];
		uncomp->source += 1;
		uncomp->source_limit = uncomp->source + RDBUFSIZ - 1;
	} else {
		uncomp->source -= RDBUFSIZ - 1;
	}

	int read =
			compressed_in->read((char*) uncomp->source - 1,
					std::min((int64_t) RDBUFSIZ,
							(int64_t) (compressed_length
									- compressed_in->tellg() - 4))).gcount();

	if (read == 0) {
		if (uncomp->source != NULL) {
			uncomp->source += RDBUFSIZ - 1;
			uncomp->source -= (compressed_length - 4) % RDBUFSIZ - 1;
			delete[] (uncomp->source - 1);
			uncomp->source = NULL;
			uncomp->source_limit = NULL;
		}
		return -1;
	}

	uncomp->source_limit = uncomp->source + read - 1;
	return uncomp->source[-1];
}

/**
 * Test decompressing a file that is too big to be stored completely in memory.
 */
void test_decompress_streaming() {
	// The size of the uncompressed file used for testing.
	const size_t FILE_SIZE = 536870912;
	// The size of the block of data to be compared when checking the file.
	const size_t TEST_COMP_SIZE = 25;
	// The size of the read and decompression buffers.
	const size_t BUFFER_SIZE = 4096;

	check_fixtures();
	prepare_compressed_file(FILE_SIZE);

	compressed_in = new std::ifstream();
	compressed_in->open(compressed_path, std::ios::in | std::ios::binary);
	TEST_ASSERT_TRUE_MESSAGE(compressed_in->is_open(),
			"Failed to open compressed file.");
	compressed_in->seekg(0, std::ios::end);
	compressed_length = compressed_in->tellg();
	compressed_in->seekg(0, std::ios::beg);

	gzip::uzlib_ungzip_wrapper unzip(&read_compressed_callback, -10);
	TEST_ASSERT_EQUAL_INT32_MESSAGE(-1, unzip.getDecompressedSize(),
			"The initial decompressed size didn't match expectations.");
	std::ifstream uncompressed_in;
	uncompressed_in.open(uncompressed_path);
	TEST_ASSERT_TRUE_MESSAGE(uncompressed_in.is_open(),
			"Failed to open uncompressed file.");

	char *uncompressed = new char[BUFFER_SIZE];
	char *decompressed = new char[BUFFER_SIZE];
	while ((size_t) uncompressed_in.tellg() < FILE_SIZE) {
		const size_t read =
				uncompressed_in.read(uncompressed, BUFFER_SIZE).gcount();
		const size_t pos = uncompressed_in.tellg();

		TEST_ASSERT_EQUAL_UINT_MESSAGE(read,
				unzip.decompress((uint8_t* ) decompressed, BUFFER_SIZE),
				"The number of decompressed bytes didn't match the decompressed size.");
		if (pos < FILE_SIZE) {
			TEST_ASSERT_FALSE_MESSAGE(unzip.done(),
					"The decompression was considered done before decompressing everything.");
			TEST_ASSERT_EQUAL_INT32_MESSAGE(-1, unzip.getDecompressedSize(),
					"The initial decompressed size didn't match expectations.");
		} else {
			TEST_ASSERT_TRUE_MESSAGE(unzip.done(),
					"The decompression wasn't considered done after decompressing everything.");
			TEST_ASSERT_EQUAL_INT32_MESSAGE(FILE_SIZE,
					unzip.getDecompressedSize(),
					"The final decompressed size didn't match expectations.");
		}

		// Compare content in blocks of TEST_COMP_SIZE characters, for better error readability.
		const size_t FILE_SIZE_LEN = log10(FILE_SIZE) + 1;
		for (size_t start = 0; start + TEST_COMP_SIZE < BUFFER_SIZE; start +=
				TEST_COMP_SIZE) {
			const size_t comp_len = std::min(TEST_COMP_SIZE,
					BUFFER_SIZE - start);
			const size_t comp_start = start + pos;
			char message[58 + FILE_SIZE_LEN * 2];
			snprintf(message, 58 + FILE_SIZE_LEN * 2,
					"Decompressed file bytes %lu-%lu don't match uncompressed file.",
					comp_start, comp_start + comp_len);
			TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(uncompressed + start,
					decompressed + start, comp_len, message);
		}
	}

	compressed_in->close();
	uncompressed_in.close();
	delete[] uncompressed;
	delete[] decompressed;
}

void test_decompress_large_wsize() {
	// The size of the uncompressed file used for testing.
	const size_t FILE_SIZE = 524288;
	// The size of the block of data to be compared when checking the file.
	const size_t TEST_COMP_SIZE = 25;

	check_fixtures();

	// Change window size from -10 to -15.
	compress_command[58] = '5';
	prepare_compressed_file(FILE_SIZE);

	compressed_in = new std::ifstream();
	compressed_in->open(compressed_path, std::ios::in | std::ios::binary);
	TEST_ASSERT_TRUE_MESSAGE(compressed_in->is_open(),
			"Failed to open compressed file.");
	char *compressed = new char[FILE_SIZE];
	compressed_length = compressed_in->read(compressed, FILE_SIZE).gcount();
	compressed_in->close();

	gzip::uzlib_ungzip_wrapper unzip((uint8_t*) compressed,
			(uint8_t*) compressed + compressed_length, -15);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(FILE_SIZE, unzip.getDecompressedSize(),
			"The initial decompressed size didn't match expectations.");
	std::ifstream uncompressed_in;
	uncompressed_in.open(uncompressed_path);
	TEST_ASSERT_TRUE_MESSAGE(uncompressed_in.is_open(),
			"Failed to open uncompressed file.");
	char *uncompressed = new char[FILE_SIZE];
	uncompressed_in.read(uncompressed, FILE_SIZE);
	uncompressed_in.close();

	char *decompressed = new char[FILE_SIZE];
	TEST_ASSERT_EQUAL_UINT_MESSAGE(FILE_SIZE,
			unzip.decompress((uint8_t* ) decompressed, FILE_SIZE),
			"The number of decompressed bytes didn't match the decompressed size.");
	TEST_ASSERT_TRUE_MESSAGE(unzip.done(),
			"The decompression wasn't considered done after decompressing everything.");
	TEST_ASSERT_EQUAL_INT32_MESSAGE(FILE_SIZE, unzip.getDecompressedSize(),
			"The final decompressed size didn't match expectations.");

	// Compare content in blocks of TEST_COMP_SIZE characters, for better error readability.
	const size_t FILE_SIZE_LEN = log10(FILE_SIZE) + 1;
	for (size_t start = 0; start + TEST_COMP_SIZE < FILE_SIZE; start +=
			TEST_COMP_SIZE) {
		const size_t comp_len = std::min(TEST_COMP_SIZE, FILE_SIZE - start);
		char message[58 + FILE_SIZE_LEN * 2];
		snprintf(message, 58 + FILE_SIZE_LEN * 2,
				"Decompressed file bytes %lu-%lu don't match uncompressed file.",
				start, start + comp_len);
		TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(uncompressed + start,
				decompressed + start, comp_len, message);
	}

	delete[] compressed;
	delete[] uncompressed;
	delete[] decompressed;
}

/**
 * The entrypoint running this test file.
 *
 * @param argc	The number of arguments.
 * @param argv	The given argument strings.
 * @return	The program exit code.
 */
int main(int argc, char **argv) {
	UNITY_BEGIN();

	RUN_TEST(test_decompress_small);
	RUN_TEST(test_decompress_large);
	RUN_TEST(test_decompress_streaming);
	RUN_TEST(test_decompress_large_wsize);

	return UNITY_END();
}
