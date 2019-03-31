// [303] In assignment #1, you will need to complete this file.  The reference
//       solution requires about 240 lines for the entire file, including
//       comments. That includes two helper functions (not included) that
//       helped reduce the total amount of boilerplate code.

#pragma once

#include <iostream>
#include <openssl/rsa.h>
#include <string>
#include <stdio.h>
#include <list>

#include "common_buf.h"
#include "common_crypto.h"
#include "common_file.h"
#include "common_net.h"
#include "protocol.h"

using namespace std;

buf_t encrypt_length(buf_t encrypted_msg, EVP_CIPHER_CTX * ctx){
  int length = encrypted_msg.num;
  string length_str = to_string(length);
  unsigned char* length_char = (unsigned char*)length_str.c_str();
  buf_t length_buf(16, length_char);

  return length_buf;
}

void send(int sd, buf_t encrypted_aes_key, buf_t encrypted_length, buf_t encrypted_msg){

  list<buf_t> list;
  list.push_back(encrypted_aes_key);
  list.push_back(buf_t("\n"));
  list.push_back(encrypted_length);
  list.push_back(buf_t("\n"));
  list.push_back(encrypted_msg);
  buf_t sum = buf_add_list(list);

  send_reliably(sd, sum);
}

/**
 * client_set() sends the SET command to set the content for a user
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 * @param setfile The file whose contents should be sent
 */
void client_set(int sd, RSA *pubkey, std::string user, std::string pass, std::string setfile) {
  // // Produce the SET command.  It looks like this:
  // //   SET\nuser_name\npassword\nnum_bytes\nbytes

  //1. enc(public_key, aes_key)\n
  buf_t aes_key = create_aes_key();
  buf_t encrypted_aes_key = make_encrypted_key(pubkey, aes_key.bytes);

  //aes context
  EVP_CIPHER_CTX * aes_context = create_aes_context(aes_key.bytes, true);

  // 2. enc(aes_key, len)\n   &  3. enc(aes_key, SET\nuser_name\npassword\nnum_bytes\nbytes)
  std::string combine;
  buf_t file_content = load_entire_file(setfile);
  combine = REQ_SET + "\n" + user + "\n" + pass + "\n" + to_string(file_content.num) + "\n" + string((char*)file_content.bytes,file_content.num);
  buf_t msg = buf_t(combine);

  // encrypt msg
  buf_t encrypted_msg = aes_crypt_msg(aes_context, msg);

  //length
  buf_t encrypted_length = encrypt_length(encrypted_msg, aes_context);

  send(sd, encrypted_aes_key, encrypted_length, encrypted_msg);
  
  free(file_content.bytes);
  free(msg.bytes);
  free(encrypted_msg.bytes);
  
  reset_aes_context(aes_context, aes_key.bytes, false);

  //decrypt the response and read
  buf_t res = reliable_get_to_eof(sd);
  buf_t decrypt_msg = aes_crypt_msg(aes_context, res);
  
  std::cout << std::string((char *)decrypt_msg.bytes, decrypt_msg.num) << std::endl;

  free(aes_key.bytes);
  free(res.bytes);
  free(decrypt_msg.bytes);

  reclaim_aes_context(aes_context);

}

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
void client_all(int sd, RSA *pubkey, std::string user, std::string pass, std::string allfile) {
  // // Produce the ALL command.  It looks like this:
  // //   ALL\nuser_name\npassword

  //1. enc(public_key, aes_key)\n
  buf_t aes_key = create_aes_key();
  buf_t encrypted_aes_key = make_encrypted_key(pubkey, aes_key.bytes);

  //aes context
  EVP_CIPHER_CTX * aes_context = create_aes_context(aes_key.bytes, true);

  //2. enc(aes_key, len)\n  &  3. enc(aes_key, ALL\nuser_name\npassword)
  std::string combine;
  combine = REQ_ALL + "\n" + user + "\n" + pass;
  buf_t msg = buf_t(combine);

  //encrypt msg
  buf_t encrypted_msg = aes_crypt_msg(aes_context, msg);
  
  //length
  buf_t encrypted_length = encrypt_length(encrypted_msg, aes_context);

  send(sd, encrypted_aes_key, encrypted_length, encrypted_msg);
  
  free(encrypted_aes_key.bytes);
  free(msg.bytes);
  free(encrypted_msg.bytes);

  reset_aes_context(aes_context, aes_key.bytes, false);

  //decrypt the response and read
  buf_t res = reliable_get_to_eof(sd);
  buf_t decrypt_msg = aes_crypt_msg(aes_context, res);
  
  std::cout << string((char *)decrypt_msg.bytes, decrypt_msg.num) << std::endl;
  write_file(allfile, (const char *)decrypt_msg.bytes, decrypt_msg.num);

  free(aes_key.bytes);
  free(res.bytes);
  free(decrypt_msg.bytes);

  reclaim_aes_context(aes_context);
}

/**
 * client_register() sends the REG command to register a new user
 *
 * @param sd      The socket descriptor for communicating with the server
 * @param pubkey  The public key of the server
 * @param user    The name of the user doing the request
 * @param pass    The password of the user doing the request
 */
void client_register(int sd, RSA *pubkey, std::string user, std::string pass) {
  // Produce the REG command.  It looks like this:
  //   REG\nuser_name\npassword

  //1. enc(public_key, aes_key)\n
  buf_t aes_key = create_aes_key();
  buf_t encrypted_aes_key = make_encrypted_key(pubkey, aes_key.bytes);

  //aes context
  EVP_CIPHER_CTX * aes_context = create_aes_context(aes_key.bytes, true);

  //2. enc(aes_key, len)\n  &  3. enc(aes_key, REG\nuser_name\npassword)
  string combine;
  combine = REQ_REG + "\n" + user + "\n" + pass;
  buf_t msg = buf_t(combine);
  
  //encrypt msg
  buf_t encrypted_msg = aes_crypt_msg(aes_context, msg);
  
  //length
  buf_t encrypted_length = encrypt_length(encrypted_msg, aes_context);

  send(sd, encrypted_aes_key, encrypted_length, encrypted_msg);

  free(encrypted_aes_key.bytes);
  free(msg.bytes);
  free(encrypted_msg.bytes);

  reset_aes_context(aes_context, aes_key.bytes, false);

  //decrypt the response and read
  buf_t res = reliable_get_to_eof(sd);
  buf_t decrypt_msg = aes_crypt_msg(aes_context, res);
  
  std::cout << std::string((char *)decrypt_msg.bytes, decrypt_msg.num) << std::endl;

  free(aes_key.bytes);
  free(res.bytes);
  free(decrypt_msg.bytes);

  reclaim_aes_context(aes_context);
}

/**
 * client_key() writes a request for the server's key on a socket descriptor.
 * When it gets it, it writes it to a file.
 *
 * @param sd      An open socket
 * @param keyfile The file to which the key should be written
 */
void client_key(int sd, std::string keyfile) {
  // Produce the KEY command.  it looks like this:
  //   KEY\n
  send_reliably(sd, buf_t(REQ_KEY + "\n"));
  buf_t res = reliable_get_to_eof(sd);
  write_file(keyfile, (const char*)res.bytes, res.num);

  free(res.bytes);
}

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
void client_get(int sd, RSA *pubkey, std::string user, std::string pass, std::string getname) {
  // // Produce the GET command.  It looks like this
  // //   GET\nuser_name\npassword\nuser_name_to_get

  //1. enc(public_key, aes_key)\n
  buf_t aes_key = create_aes_key();
  buf_t encrypted_aes_key = make_encrypted_key(pubkey, aes_key.bytes);

  //aes context
  EVP_CIPHER_CTX * aes_context = create_aes_context(aes_key.bytes, true);

  //2. enc(aes_key, len)\n  &  3. enc(aes_key, GET\nuser_name\npassword\nuser_name_to_get)
  std::string combine;
  combine = REQ_GET + "\n" + user + "\n" + pass + "\n" + getname;
  buf_t msg = buf_t(combine);

  //encrypt msg
  buf_t encrypted_msg = aes_crypt_msg(aes_context, msg);
  
  //length
  buf_t encrypted_length = encrypt_length(encrypted_msg, aes_context);
  
  send(sd, encrypted_aes_key, encrypted_length, encrypted_msg);

  free(encrypted_aes_key.bytes);
  free(msg.bytes);
  free(encrypted_msg.bytes);
  
  reset_aes_context(aes_context, aes_key.bytes, false);

  //decrypt the response and read
  buf_t res = reliable_get_to_eof(sd);
  buf_t decrypt_msg = aes_crypt_msg(aes_context, res);
  
  std::cout << string((char *)decrypt_msg.bytes, decrypt_msg.num) << std::endl;
  string filename = getname+".file.dat";
  write_file(filename, (const char *)decrypt_msg.bytes, decrypt_msg.num);

  free(aes_key.bytes);
  free(res.bytes);
  free(decrypt_msg.bytes);

  reclaim_aes_context(aes_context);
}

/**
 * client_bye() writes a request for the server to exit.
 *
 * @param sd An open socket
 */
void client_bye(int sd) {
  // Produce the BYE command.  it looks like this:
  //   BYE\n
  send_reliably(sd, buf_t(REQ_BYE + "\n"));

  buf_t res = reliable_get_to_eof(sd);
  std::cout << string((char *)res.bytes, res.num) << std::endl;

  free(res.bytes);
}
