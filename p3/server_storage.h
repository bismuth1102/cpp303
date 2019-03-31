// [303] In assignment #2, this file will need to change substantially from how
//       it was in assignment #1.  The Directory can no longer use a std::map as
//       its root data structure, since the std::map does not have fine-grained
//       concurrency.  Instead, a vector of vectors should be used.  Second,
//       that vector of vectors will require mutexes for fine-grained
//       concurrency.  There should be a fixed number of entries in the first
//       vector, and the code should use std::hash to hash user names to
//       positions.  Each entry should have a map for its key/value store.  The
//       persistence strategy should become explicit, only happening in response
//       to a SAV command, and should persist the entire directory.  Finally,
//       everything must be thread safe. This includes the use of two-phase
//       locking for the ALL and SAV commands. The reference solution is about
//       647 lines in total.

// [303] In assignment #3, this file will again need to change substantially.
//       The new challenge is to do fine-grained persistence.  That is, we do
//       not want to write the *whole* file any time *anything* changes.
//       Instead, we want to incrementally modify the file.  Our strategy will
//       be to use something like a "log-structured file system" (LFS).  We can
//       think about the Directory as actually embodying two things: a data
//       structure for finding data, and the data itself.  It is not necessary
//       to persist the exact data structure, as long as (1) the data itself is
//       persisted, and (2) upon any server shut-down, the data structure can be
//       re-created from the persisted data.
//
//       To that end, our LFS-inspired Directory will introduce a few new entry
//       types, in addition to the previous DIRENTRY and K.VENTRY.  These will
//       represent incremental changes to the data structure.  The new entries
//       are:
//
//         - DIRCNTNT - to indicate a new content field for a user
//         - K.VRMKEY - to indicate that a key/value pair should be removed
//         - K.VINKEY - to indicate that a new key/value pair should be added
//         - K.VUPKEY - to indicate that the value for a key should be changed
//
//       Note that these require the user name to be given.  That is, their form
//       will be like this:
//
//         K.VRMKEYl1ul2k
//         K.VINKEYl1ul2kl3v
//         K.VUPKEYl1ul2kl3v
//         DIRCNTNTl1ul4c
//
//       Where l1 is the binary representation of the length of the user name, u
//       is the binary representation of the user name, l2 is the binary
//       representation of the length of the key, k is the binary
//       representation of the key, l3 is the binary representation of the
//       length of the value, v is the binary representation of the value, l4 is
//       the binary representation of the length of a user's content, and c is
//       the binary representation of a user's content.
//
//       As you can probably imagine, the log can get outrageously and
//       unnecessarily long.  To avoid such problems, you should continue to
//       support the SAV command, so that the data structure can be periodically
//       re-persisted in a more compact form.  Note that you must handle
//       concurrency differently than in the previous assignment.

#pragma once

#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <openssl/md5.h>
#include <unordered_map>
#include <vector>

#include "common_buf.h"
#include "common_err.h"
// NB: It's a really bad idea to include protocol.h in this file, and to then
//     use the strings it defines as the way to return error messages.  We're
//     doing it just to save some code in other parts of the assignment.  A real
//     server should not intertwine its data structures and its protocol.
#include "protocol.h"

using namespace std;

/**
 * Directory wraps a vector of vectors, and provides functions that correspond
 * 1:1 with the data requests that a client can make.  In that manner, the
 * server command handlers need only parse a request, send its parts to the
 * Directory, and then format and return the Directory's result.
 *
 * The directory is a persistent object.  For the time being, persistence is
 * achieved by writing the entire directory to disk in response to requests from
 * the client. We use a relatively simple binary wire format for the directory:
 *
 *  - Each user entry begins with the magic 8-byte constant DIRENTRY
 *  - Next comes a 4-byte binary write of the length of the username
 *  - Then a binary write of the bytes of the username
 *  - Then a binary write of the length of pass_hash
 *  - Then a binary write of the bytes of pass_hash
 *  - Then a binary write of num_bytes
 *  - Then, if num_bytes > 0, a binary write of the bytes field
 *  - If there are key/value pairs, then each is written as follows:
 *    - First, the magic 8-byte constant K.VENTRY
 *    - Then a binary write of the length of the key
 *    - Then a binary write of the bytes of the key
 *    - Then a binary write of the length of the value
 *    - Then a binary write of the bytes of the value
 *
 * This is repeated for each Entry in the Directory.
 */
class Directory {
public:
  /** Entry represents one user in the Directory */
  struct Entry {
    /**
     * The name of the user; max 64 characters, never changes after
     * initialization
     */
    std::string username;

    /** The hashed password.  Note that the password is a max of 128 chars */
    std::string pass_hash;

    /** The number of bytes for the user's content */
    int num_bytes;

    /** The raw bytes of the user's content */
    unsigned char *bytes;

    /** The user's key/value store */
    std::unordered_map<std::string, buf_t> map;
  public:
    Entry(string _username, string _pass_hash, int _num_bytes, unsigned char *_bytes, unordered_map<string, buf_t> _map){
      username = _username;
      pass_hash = _pass_hash;
      num_bytes = _num_bytes;
      bytes = _bytes;
      map = _map;
    }

    Entry(string _username, string _pass_hash, int _num_bytes, unsigned char *_bytes){
      username = _username;
      pass_hash = _pass_hash;
      num_bytes = _num_bytes;
      bytes = _bytes;
    }

    Entry(string _username, string _pass_hash){
      username = _username;
      pass_hash = _pass_hash;
    }

    Entry(int _num_bytes){
      num_bytes = _num_bytes;
    }
  };

  struct hashList {
    int index;
    mutex lock;
    vector<Entry> row;
  public:
    hashList(int _index){
      index = _index;
    }
  };

  /**
   * A unique 8-byte code to use as a prefix each time an Entry is written to
   * disk.
   *
   * NB: this isn't needed in assignment 1, but will be useful for backwards
   *     compatibility later on.
   */
  static constexpr const char *DIRENTRY = "DIRENTRY";

  /**
   * A unique 8-byte code to use as a prefix each time a key/value pair is
   * written to disk.
   */
  static constexpr const char *KVENTRY = "K.VENTRY";

  /**
   * Since std::map is not thread-safe, we will make the directory into a
   * bucket-of-vectors hash table.  This is the bucket count.
   */
  static const int BUCKETS = 1024;

  /**
   * filename is the name of the file from which the directory was loaded, and
   * to which we persist the directory every time it changes
   */
  std::string filename = "";

  static vector<hashList* > col;

  void initCol();

  Entry loadDirentry(FILE *f);

  pair<string, buf_t> loadK_Ventry(FILE *f, Entry e);

  /**
   * Write the entire directory to the file specified by this.filename.  To
   * ensure durability, the directory must be persisted in two steps.  First, it
   * must be written to a temporary file (this.filename.tmp).  Then the
   * temporary file can be renamed to replace the older version of the
   * directory.
   */
  void persist();

public:
  /**
   * Construct an empty directory and specify the file from which the directory
   * should be loaded.  To avoid exceptions and errors in the constructor, the
   * act of loading data is separate from construction.
   */
  Directory(std::string fname);

  /**
   * Populate the directory by loading this.filename.  Note that load() begins
   * by clearing the directory, so that when the call is complete, exactly and
   * only the contents of the file are in the directory.
   *
   * @returns False if any error is encountered, and true otherwise
   */

  bool load();

  /**
   * Create a new entry in the Directory.  If the user_name already exists, we
   * should return an error.  Otherwise, hash the password, and then save an
   * entry with the username, hashed password, and a zero-byte content.
   *
   * @param user_name The user name to register
   * @param pass  The password to associate with that user name
   *
   * @returns False if the username already exists, true otherwise
   */
  bool add_user(std::string user_name, std::string pass);

  /**
   * Set the data bytes for a user, but do so if and only if the password
   * matches
   *
   * @param user_name The name of the user whose content is being set
   * @param pass The password for the user, used to authenticate
   * @param num_bytes The length of the array of bytes being provided
   * @param bytes The actual bytes to add to the user's Entry
   *
   * @returns A buf_t with a message that is the result of the attempt
   */
  buf_t set_user_data(std::string user_name, std::string pass, int num_bytes,
                      unsigned char *bytes);
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
                      std::string who);

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
  buf_t get_all_users(std::string user_name, std::string pass);

  /**
   * Perform an insertion into the user's key/value store
   *
   * @param user_name The name of the user who made the request
   * @param pass The password for the user, used to authenticate
   * @param key  The key to insert
   * @param val  The bytes of value to insert
   *
   * @returns A buf_t with a message that is the result of the attempt.
   */
  buf_t kvinsert(std::string user_name, std::string pass, std::string key,
                 buf_t val);

  /**
   * Perform an update in the user's key/value store
   *
   * @param user_name The name of the user who made the request
   * @param pass The password for the user, used to authenticate
   * @param key  The key to update
   * @param val  The bytes of value to update
   *
   * @returns A buf_t with a message that is the result of the attempt
   */
  buf_t kvupdate(std::string user_name, std::string pass, std::string key,
                 buf_t val);

  /**
   * Perform a get from the user's key/value store
   *
   * @param user_name The name of the user who made the request
   * @param pass The password for the user, used to authenticate
   * @param key  The key to get
   *
   * @returns A buf_t with the value, or a buf_t with a negative number on
   *          failure.  The num will be -1 for invalid user_name, -2 for bad
   *          password, and -3 for invalid key.
   */
  buf_t kvget(std::string user_name, std::string pass, std::string key);

  /**
   * Perform a delete in the user's key/value store
   *
   * @param user_name The name of the user who made the request
   * @param pass The password for the user, used to authenticate
   * @param key  The key to delete
   *
   * @returns A buf_t with the result of the attempt.
   */
  buf_t kvdel(std::string user_name, std::string pass, std::string key);

  /**
   * Public interface for saving the directory to a file
   */
  void save();
};
