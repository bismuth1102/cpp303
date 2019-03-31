#pragma once

#include <openssl/rsa.h>
#include <string>

/**
 * client_set() sends the SET command to set the content for a user
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 * @param setfile The file whose contents should be sent
 */
void client_set(int sd, RSA *pubkey, std::string user, std::string pass,
                std::string setfile);

/**
 * client_all() sends the ALL command to get a listing of all users, formatted
 * as text with one entry per line.
 *
 * @param sd The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user The name of the user doing the request
 * @param pass The password of the user doing the request
 * @param allfile The file where the result should go
 */
void client_all(int sd, RSA *pubkey, std::string user, std::string pass,
                std::string allfile);

/**
 * client_register() sends the REG command to register a new user
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 */
void client_register(int sd, RSA *pubkey, std::string user, std::string pass);
/**
 * client_key() writes a request for the server's key on a socket descriptor.
 * When it gets it, it writes it to a file.
 *
 * @param sd      An open socket
 * @param keyfile The file to which the key should be written
 */
void client_key(int sd, std::string keyfile);

/**
 * client_bye() writes a request for the server to exit.
 *
 * @param sd An open socket
 */
void client_bye(int sd);

/**
 * client_get() requests the content associated with a user, and saves it to a
 * file called <user>.file.dat.
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 * @param getname The name of the user whose content should be fetched
 */
void client_get(int sd, RSA *pubkey, std::string user, std::string pass,
                std::string getname);

/**
 * client_kvinsert() attempts to create a new mapping from a key to a value.
 * Both key and value are read from files.  If the key already exists, then the
 * server will return an error.
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 * @param key     The key for this command
 * @param vfile   The file holding the bytes that represent the value
 */
void client_kvinsert(int sd, RSA *pubkey, std::string user, std::string pass,
                     std::string key, std::string vfile);
/**
 * client_kvupdate() attempts to update the value to which a key is mapped. Both
 * key and value are read from files.  If the key does not already exist, then
 * the server will return an error.
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 * @param key     The key for this command
 * @param vfile   The file holding the bytes that represent the value
 */
void client_kvupdate(int sd, RSA *pubkey, std::string user, std::string pass,
                     std::string key, std::string vfile);

/**
 * client_kvdelete() attempts to delete a key/value mapping.  If the key does
 * not exist, then the server will return an error.
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 * @param key     The key for this command
 */
void client_kvdelete(int sd, RSA *pubkey, std::string user, std::string pass,
                     std::string key);

/**
 * client_kvget() requests the value for a key, and saves it to a file
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 * @param key     The key for this command
 * @param vfile   The file where the value should go
 */
void client_kvget(int sd, RSA *pubkey, std::string user, std::string pass,
                  std::string key, std::string vfile);

/**
 * client_save() instructs the server to persist its data
 *
 * @param sd An open socket
 */
void client_save(int sd);