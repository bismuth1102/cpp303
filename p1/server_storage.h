// [303] In assignment #1, you will need to complete get_all_users(), which
//       requires about 33 lines of code, and persist(), which requires about 30
//       lines of code.  Note that you should not modify load().  That is, you
//       will know that persist() is correct when it works correctly with
//       load().

#pragma once

#include <cstring>
#include <iostream>
#include <openssl/md5.h>
#include <unordered_map>

#include "common_buf.h"
#include "common_err.h"
// NB: It's a really bad idea to include protocol.h in this file, and to then
//     use the strings it defines as the way to return error messages.  We're
//     doing it just to save some code in other parts of the assignment.  A real
//     server should not intertwine its data structures and its protocol.
#include "protocol.h"

using namespace std;

/**
 * Directory wraps a std::unordered_map, and provides functions that correspond
 * 1:1 with the data requests that a client can make.  In that manner, the
 * server command handlers need only parse a request, send its parts to the
 * Directory, and then format and return the Directory's result.
 *
 * The directory is a persistent object.  For the time being, persistence is
 * achieved by writing the entire directory to disk every time it is modified.
 * We use a relatively simple binary wire format for the directory:
 *
 *  - Each entry begins with the magic 8-byte constant DIRENTRY
 *  - Next comes a 4-byte binary write of the length of the username
 *  - Then a binary write of the bytes of the username
 *  - Then a binary write of the length of pass_hash
 *  - Then a binary write of the bytes of pass_hash
 *  - Then a binary write of num_bytes
 *  - Finally, if num_bytes > 0, a binary write of the bytes field
 *
 * This is repeated for each Entry in the Directory.
 */
class Directory {
  /** Entry represents one user in the Directory */
  struct Entry {
    /** The name of the user; max 64 characters */
    std::string username;

    /** The hashed password.  Note that the password is a max of 128 chars */
    std::string pass_hash;

    /** The number of bytes for the user's content */
    int num_bytes;

    /** The raw bytes of the user's content */
    unsigned char *bytes;
  };

  /**
   * A unique 8-byte code to use as a prefix each time an Entry is written to
   * disk.
   *
   * NB: this isn't needed in assignment 1, but will be useful for backwards
   *     compatibility later on.
   */
  static constexpr const char *DIRENTRY = "DIRENTRY";

  /** directory is the map of entries, indexed by username */
  std::unordered_map<std::string, Entry> directory;

  /**
   * filename is the name of the file from which the directory was loaded, and
   * to which we persist the directory every time it changes
   */
  std::string filename = "";

  /**
   * Write the entire directory to the file specified by this.filename.  To
   * ensure durability, the directory must be persisted in two steps.  First, it
   * must be written to a temporary file (this.filename.tmp).  Then the
   * temporary file can be renamed to replace the older version of the
   * directory.
   */
  void persist() {
    // [303] This function should require about 30 lines of code, and should
    //       operate in a manner that is consistent with load(), below.
    string new_filename = this->filename + ".tmp";
    FILE *f = fopen(new_filename.c_str(), "w");

    for(auto i : this->directory) {
      fwrite("DIRENTRY", sizeof(char), 8, f);

      string username = i.second.username;
      int username_length = username.length();

      string password = i.second.pass_hash;
      int password_length = password.length();

      int num_bytes = i.second.num_bytes;
      unsigned char *bytes = i.second.bytes;

      fwrite(to_string(username_length).c_str(), sizeof(int), 1, f);
      fwrite(username.c_str(), sizeof(char), username_length, f);
      fwrite(to_string(password_length).c_str(), sizeof(int), 1, f);
      fwrite(password.c_str(), sizeof(char), password_length, f);
      if(num_bytes>0){
        fwrite(to_string(num_bytes).c_str(), sizeof(int), 1, f);
        fwrite(bytes, sizeof(char), num_bytes, f);
      }
    }
    fclose(f);

    int result = rename(new_filename.c_str(), this->filename.c_str());
    if(result!=0){
      std::cerr << "rename error" << std::endl;
    }
  }

public:
  /**
   * Construct an empty directory and specify the file from which the directory
   * should be loaded.  To avoid exceptions and errors in the constructor, the
   * act of loading data is separate from construction.
   */
  Directory(std::string fname) : filename(fname) {}

  /**
   * Populate the directory by loading this.filename.  Note that load() begins
   * by clearing the directory, so that when the call is complete, exactly and
   * only the contents of the file are in the directory.
   *
   * @returns False if any error is encountered, and true otherwise
   */
  bool load() {
    cout << "load" << endl;
    FILE *f = fopen(filename.c_str(), "r");
    if (f == nullptr) {
      std::cerr << "File not found: " << filename << std::endl;
      return false;
    }
    // read while there's something to read:
    char pack_buf[9] = {0};
    while (fread(pack_buf, sizeof(char), 8, f)) {
      // Make sure we got a directory entry:
      char name_buf[65] = {0}, pass_buf[129] = {0};
      unsigned size = 0;
      std::string packet(pack_buf);
      if (packet != "DIRENTRY") {
        std::cerr << "Invalid packet type: " << packet << std::endl;
        directory.clear();
        return false;
      }

      // Read the fields
      if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        std::cerr << "File read error\n";
        directory.clear();
        return false;
      }
      if (size != fread(name_buf, sizeof(char), size, f)) { //size bytes
        std::cerr << "File read error\n";
        directory.clear();
        return false;
      }
      std::string name(name_buf);
      if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        std::cerr << "File read error\n";
        directory.clear();
        return false;
      }
      if (size != fread(pass_buf, sizeof(char), size, f)) { //size bytes
        std::cerr << "File read error\n";
        directory.clear();
        return false;
      }
      std::string pass(pass_buf);
      if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        std::cerr << "File read error\n";
        directory.clear();
        return false;
      }
      unsigned char *byte_buf = nullptr;
      if (size) {
        byte_buf = (unsigned char *)malloc(size);
        if (size != fread(byte_buf, sizeof(char), size, f)) { //size bytes
          std::cerr << "File read error\n";
          directory.clear();
          return false;
        }
      }

      // Put the entry in the directory
      Entry e = {name, pass, (int)size, byte_buf};
      this->directory.insert({name, e});
      free(byte_buf);
    }
    fclose(f);
    cout << "finish load" << endl;
    return true;
  }

  /**
   * Create a new entry in the Directory.  If the user_name already exists, we
   * should return an error.  Otherwise, hash the password, and then save an
   * entry with the username, hashed password, and a zero-byte content.
   *
   * Note: this modifies the directory, so it must persist.
   *
   * @param user_name The user name to register
   * @param pass  The password to associate with that user name
   *
   * @returns False if the username already exists, true otherwise
   */
  bool add_user(std::string user_name, std::string pass) {
    // If the user exists, fail
    if (directory.find(user_name) != directory.end())
      return false;

    // Remember: never store passwords.  Store their hashes instead
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    // Be sure to persist the directory since we are modifying it
    Entry e = {user_name, std::string((char *)result), 0, nullptr};
    directory.insert({user_name, e});
    persist();
    return true;
  }

  /**
   * Set the data bytes for a user, but do so if and only if the password
   * matches
   *
   * Note: this modifies the directory, so it must persist.
   *
   * @param user_name The name of the user whose content is being set
   * @param pass The password for the user, used to authenticate
   * @param num_bytes The length of the array of bytes being provided
   * @param bytes The actual bytes to add to the user's Entry
   *
   * @returns A buf_t with a message that is the result of the attempt
   */
  buf_t set_user_data(std::string user_name, std::string pass, int num_bytes,
                      unsigned char *bytes) {
    // Authenticate the user via password
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);
    auto i = directory.find(user_name);
    if (i == directory.end())
      return buf_t(RES_ERR_USER_EXISTS);
    if (i->second.pass_hash != std::string((char *)result))
      return buf_t(RES_ERR_LOGIN);

    // Copy the data to a new region, and put it into the directory
    unsigned char *data = (unsigned char *)malloc(num_bytes);
    memcpy(data, bytes, num_bytes);
    i->second.num_bytes = num_bytes;
    i->second.bytes = data;
    // Be sure to persist before returning
    persist();
    return buf_t(RES_OK);
  }

  /**
   * Return a copy of the user data for a user, but do so only if the password
   * matches
   *
   * @param user_name The name of the user who made the request
   * @param pass The password for the user, used to authenticate
   * @param who The name of the user whose content is being fetched
   *
   * @returns A buf_t with the data, or a buf_t with a negative number on
   *          failure.  The num will be -1 for invalid user_name, -2 for bad
   *          password, and -3 for invalid who parameter.  Note that num can be
   *          0 if the user has no data.
   */
  buf_t get_user_data(std::string user_name, std::string pass,
                      std::string who) {
    // Authenticate the user via password
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);
    auto i = directory.find(user_name);
    if (i == directory.end())
      return {-1, nullptr};
    if (i->second.pass_hash != std::string((char *)result))
      return {-2, nullptr};

    // Find the user, copy the user's data and return it
    auto w = directory.find(who);
    if (i == directory.end())
      return {-3, nullptr};
    buf_t res(w->second.num_bytes);
    memcpy(res.bytes, w->second.bytes, res.num);
    return res;
  }

  /**
   * Return a newline-delimited string containing all of the usernames in the
   * directory
   *
   * @param user_name The name of the user who made the request
   * @param pass The password for the user, used to authenticate
   *
   * @returns A buf_t with the data, or a buf_t with a negative number on
   *          failure.  The num will be -1 for invalid user_name, or -2 for bad
   *          password
   */
  buf_t get_all_users(std::string user_name, std::string pass) {
  // [303] This function requires about 33 lines of code
  // Authenticate the user via password
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);
    auto i = directory.find(user_name);
    if (i == directory.end())
      return buf_t(RES_ERR_USER_EXISTS);
    if (i->second.pass_hash != std::string((char *)result))
      return buf_t(RES_ERR_LOGIN);

    // Find the user, copy all the user's data and return it
    string all_username;
    buf_t res;
    for (auto x : directory){
      auto w = x.first;
      all_username = all_username + w + "\n";
    }
    // enc(aes_key, OK\nnum_bytes\nuser_0\nuser_1\nuser_2...)<EOF>
    res = buf_t(RES_OK + "\n" + to_string(all_username.length()) + "\n" + all_username);
    return res;
  }
};
