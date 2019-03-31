// [303] In assignment #1, you will need to complete this file.  The reference
//       solution requires about 240 lines for the entire file, including
//       comments. That includes four helper functions (not included) that
//       helped reduce the total amount of boilerplate code.

#pragma once

#include <string>

#include "common_buf.h"
#include "common_crypto.h"
#include "common_net.h"
#include "protocol.h"
#include "server_storage.h"
#include <vector>
#include <sstream>

using namespace std;

istringstream buf_t2sstream(buf_t buf){
  istringstream input(string((char *)buf.bytes, buf.num));
  return input;
}

/**
 * Respond to an ALL command by generating a list of all the usernames in the
 * directory and returning them, one per line.
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_all(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req) {
	cout << "all" << endl;

	istringstream input = buf_t2sstream(req);

	string command;
	string user_name;
	string password;

	getline(input, command);
	getline(input, user_name);
	getline(input, password);

	buf_t res = dir->get_all_users(user_name, password);
	buf_t encrypted_res = aes_crypt_msg(ctx, res);

	send_reliably(sd, encrypted_res);

	free(res.bytes);
	free(encrypted_res.bytes);

	reclaim_aes_context(ctx);
}

/**
 * Respond to a SET command by putting the provided data into the directory
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_set(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req) {
	cout << "set" << endl;

	istringstream input = buf_t2sstream(req);

	string command;
	string user_name;
	string password;
	string str_num_bytes;
	string str_bytes;

	getline(input, command);
	getline(input, user_name);
	getline(input, password);
	getline(input, str_num_bytes);
	getline(input, str_bytes);

	cout << "set content " << str_bytes << endl;

	int num_bytes = stoi(str_num_bytes);
	buf_t bytes = buf_t(str_bytes);

	buf_t res = dir->set_user_data(user_name, password, num_bytes, bytes.bytes);
	buf_t encrypted_res = aes_crypt_msg(ctx, res);

	send_reliably(sd, encrypted_res);

	free(res.bytes);
	free(encrypted_res.bytes);
	free(bytes.bytes);
	
	reclaim_aes_context(ctx);
}

/**
 * Respond to a GET command by getting the data for a user
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_get(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req) {
	cout << "get" << endl;

	istringstream input = buf_t2sstream(req);

	string command;
	string user_name;
	string password;
	string who;

	getline(input, command);
	getline(input, user_name);
	getline(input, password);
	getline(input, who);

	cout << "who " << who << endl;

	buf_t res = dir->get_user_data(user_name, password, who);
	buf_t encrypted_res;
	if(res.num==-1){
		buf_t err(RES_ERR_USER_EXISTS);
		encrypted_res = aes_crypt_msg(ctx, err);
		free(err.bytes);
	}
	else if(res.num==-2){
		buf_t err(RES_ERR_LOGIN);
		encrypted_res = aes_crypt_msg(ctx, err);
		free(err.bytes);
	}
	else if(res.num==-3){
		buf_t err(RES_ERR_USER_EXISTS);
		encrypted_res = aes_crypt_msg(ctx, err);
		free(err.bytes);
	}
	else{
		string combine = "OK" + to_string(res.num) + string((char*)res.bytes,res.num);
		buf_t ok_res = buf_t(combine);
		encrypted_res = aes_crypt_msg(ctx, ok_res);
		free(ok_res.bytes);
	}

	send_reliably(sd, encrypted_res);

	free(res.bytes);
	free(encrypted_res.bytes);

	reclaim_aes_context(ctx);
}

/**
 * Respond to a REG command by trying to add a new user
 *
 * @param sd  The socket onto which the result should be written
 * @param dir The directory data structure
 * @param ctx The AES encryption context
 * @param req The unencrypted contents of the request
 */
void server_cmd_reg(int sd, Directory *dir, EVP_CIPHER_CTX *ctx, buf_t req) {
	cout << "reg" << endl;

	istringstream input = buf_t2sstream(req);

	string command;
	string user_name;
	string password;

	getline(input, command);
	getline(input, user_name);
	getline(input, password);

	cout << user_name << " " << password << endl;

	bool bool_reg = dir->add_user(user_name, password);
	buf_t encrypted_res;
	if(bool_reg){
		cout << "reg ok" << endl;
		buf_t res = buf_t("OK");
		
		encrypted_res = aes_crypt_msg(ctx, res);
		free(res.bytes);
	}
	else{
		cout << "reg error" << endl;
		encrypted_res = aes_crypt_msg(ctx, {-1, nullptr});
	}

	send_reliably(sd, encrypted_res);

	free(encrypted_res.bytes);

	reclaim_aes_context(ctx);
	
}

/**
 * In response to a request for a key, do a reliable send of the contents of the
 * pubfile
 *
 * @param sd The socket on which to write the pubfile
 * @param pubfile A pair consisting of pubfile length and contents
 */
void server_cmd_key(int sd, buf_t pubfile) {

  send_reliably(sd, pubfile);

}