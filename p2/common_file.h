#pragma once

#include <string>

#include "common_buf.h"

/**
 * Determine if a file exists.  Note that using this is not a good way to avoid
 * TOCTOU bugs, but it is acceptable for this assignment.
 *
 * @param filename The name of the file whose existence is being checked
 *
 * @returns true if the file exists, false otherwise
 */
bool file_exists(std::string filename);

/**
 * Load a file and return its contents, along with a number indicating the file
 * size.
 *
 * @param filename The name of the file to open
 *
 * @returns A buf_t with the contents of the file, or a -1 on error
 */
buf_t load_entire_file(std::string filename);

/**
 * Create or truncate a file and populate it with the provided data
 *
 * @param filename The name of the file to create/truncate
 * @param data     The actual data to write
 * @param bytes    The number of bytes of data to write
 *
 * @returns false on error, true if the file was written in full
 */
bool write_file(std::string filename, const char *data, size_t bytes);