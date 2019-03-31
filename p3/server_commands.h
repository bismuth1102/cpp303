#pragma once

#include <openssl/pem.h>

#include "common_buf.h"
#include "server_storage.h"

/**
 * Respond to an ALL command by generating a list of all the usernames in the
 * directory and returning them, one per line.
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_all(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * Respond to a SET command by putting the provided data into the directory
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_set(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * Respond to a GET command by getting the data for a user
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_get(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * Respond to a REG command by trying to add a new user
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_reg(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * In response to a request for a key, do a reliable send of the contents of the
 * pubfile
 *
 * @param sd The socket on which to write the pubfile
 * @param pubfile A pair consisting of pubfile length and contents
 */
void server_cmd_key(int sd, buf_t pubfile);

/**
 * Respond to a keyins command by trying to insert a new key/value pair
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_keyins(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * Respond to a keyget command by trying to do a get from the key/value store
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_keyget(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * Respond to a keydel command by trying to delete a key/value pair
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_keydel(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * Respond to a keyupd command by trying to update a key/value pair
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_keyupd(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req);

/**
 * Respond to a SAVE command
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_save(int sd, Directory *dir);