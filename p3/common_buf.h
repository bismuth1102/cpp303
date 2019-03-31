// [303] This file should not require any modifications for assignment #1
// [303] This file did not change between assignments #1 and #2
// [303] This file should not require any modifications for assignment #2

#pragma once

#include <cstring>
#include <string>

/**
 * buf_t is a tuple, consisting of a stream of bytes, and a count of how many
 * bytes are in the stream.  We use buf_t instead of std::pair, so that it is
 * easier to be consistent about:
 *
 *   - order of num and bytes
 *   - using signed 32-bit size
 *   - using unsigned char*
 *
 * NB: Even though it is a struct, buf_t is small enough that it is OK to pass
 *     it by value, instead of by reference.
 */
struct buf_t {
  /** The number of bytes in the buffer */
  int num;

  /** The bytes of content in the buffer */
  unsigned char *bytes;

  /** basic constructor */
  buf_t() : num(-1), bytes(nullptr) {}

  /**
   * Construct from existing data, WITHOUT COPYING
   *
   * @param _num   The number of bytes in the provided array
   * @param _bytes An array of bytes to use in this buf_t
   */
  buf_t(int _num, unsigned char *_bytes) : num(_num), bytes(_bytes) {}

  /**
   * Construct from a string
   *
   * @param msg The string to use to create this buf_t
   */
  buf_t(std::string msg) {
    this->num = (int)msg.length();
    this->bytes = (unsigned char *)malloc(this->num + 1);
    memset(this->bytes, 0, this->num + 1);
    memcpy(this->bytes, msg.c_str(), msg.length());
  }

  /**
   * Construct by allocating and zeroing the buffer
   *
   * @param _num The number of bytes in the new buf_t
   */
  buf_t(int _num) : num(_num) {
    this->bytes = (unsigned char *)malloc(_num + 1);
    memset(bytes, 0, num + 1);
  }
};