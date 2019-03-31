// [303] This file should not require any modifications for assignment #1

#pragma once

#include <cstring>
#include <string>
#include <list>

using namespace std;

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

buf_t buf_add(buf_t a, buf_t b){
  int res_num = a.num + b.num;
  unsigned char * res_bytes = (unsigned char *)malloc(res_num + 1);
  memset(res_bytes, 0, res_num + 1);
  memcpy(res_bytes, a.bytes, a.num);
  memcpy(res_bytes+a.num, b.bytes, b.num);
  buf_t res(res_num, res_bytes);
  return res;
}


buf_t buf_add_list(list<buf_t> buf_list){
  buf_t sum(0);
  for(list<buf_t>::iterator it = buf_list.begin(); it != buf_list.end(); ++it){
    sum = buf_add(sum, *it);
  }
  return sum;
}
