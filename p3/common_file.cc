#include <iostream>
#include <string>
#include <sys/stat.h>

#include "common_buf.h"
#include "common_err.h"

/**
 * Determine if a file exists.  Note that using this is not a good way to avoid
 * TOCTOU bugs, but it is acceptable for this assignment.
 *
 * @param filename The name of the file whose existence is being checked
 *
 * @returns true if the file exists, false otherwise
 */
bool file_exists(std::string filename) {
  struct stat stat_buf;
  return (stat(filename.c_str(), &stat_buf) == 0);
}

/**
 * Load a file and return its contents, along with a number indicating the file
 * size.
 *
 * @param filename The name of the file to open
 *
 * @returns A buf_t with the contents of the file, or a -1 on error
 */
buf_t load_entire_file(std::string filename) {
  // make sure file exists.  stat also lets us get its size later
  struct stat stat_buf;
  if (stat(filename.c_str(), &stat_buf) != 0) {
    std::cerr << "File " << filename << " not found\n";
    return {-1, nullptr};
  }

  // open the file, make space for the previous # bytes (TOCTOU risk)
  FILE *f = fopen(filename.c_str(), "rb");
  if (!f) {
    std::cerr << "File " << filename << " not found\n";
    return {-1, nullptr};
  }

  // NB: Reading one extra byte should mean we get an EOF and only size bytes.
  //     Any error will cause us to crash.
  // NB: since we know it's a true file, we don't need to worry about short
  //     EINTR.
  buf_t res(stat_buf.st_size + 1);
  int recd = fread(res.bytes, sizeof(char), res.num, f);
  --res.num; // so caller has right number of usable bytes
  if (recd != res.num || !feof(f)) {
    std::cerr << "Wrong # bytes reading " << filename << std::endl;
    free(res.bytes);
    fclose(f);
    return {-1, nullptr};
  }
  fclose(f);
  return res;
}

/**
 * Create or truncate a file and populate it with the provided data
 *
 * @param filename The name of the file to create/truncate
 * @param data     The actual data to write
 * @param bytes    The number of bytes of data to write
 *
 * @returns false on error, true if the file was written in full
 */
bool write_file(std::string filename, const char *data, size_t bytes) {
  FILE *f = fopen(filename.c_str(), "wb");
  if (f == nullptr) {
    std::cerr << "Unable to open '" << filename << "' for writing\n";
    return false;
  }
  // NB: since we know it's a true file, we don't need to worry about short
  //     EINTR.
  if (fwrite(data, sizeof(char), bytes, f) != bytes) {
    fclose(f);
    std::cerr << "Incorrect number of bytes written to " << filename
              << std::endl;
    return false;
  }
  fclose(f);
  return true;
}