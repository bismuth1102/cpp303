// [303] In assignment #1, you will need to complete serve_client()

#pragma once

#include <iostream>
#include <openssl/rsa.h>

#include "common_buf.h"
#include "common_crypto.h"
#include "common_net.h"
#include "protocol.h"
#include "server_commands.h"
#include "server_storage.h"
#include <vector>
#include <sstream>

using namespace std;


/** In request format #1, the first line is 4 bytes */
static const int LINE1_FMT1_LEN = 4;

/** In request format #2, the first line is 256 bytes */
static const int LINE1_FMT2_LEN = 256;

/** In request format #2, there is a second line, and it is 16 bytes */
static const int LINE2_LEN = 16;

/**
 * When a new client connection is accepted, this code will run to figure out
 * what the client is requesting, and to dispatch to the right function for
 * satisfying the request.
 *
 * @param sd  The socket on which communication with the client takes place
 * @param pri The private key used by the server
 * @param pub The public key file contents, to send to the client
 * @param dir The directory with which clients interact
 *
 * @returns true if the server should halt immediately, false otherwise
 */
bool serve_client(int sd, RSA *pri, buf_t pub, Directory *dir) {
  // [303] The final version of this function will probably be around 150 lines
  //       of code.  This is *extremely* tedious code, because it needs to get
  //       data from the network without ever getting stuck (e.g., because it
  //       reads more than a correct client should send).

  // There are two message formats that the client can send.  The first format
  // is "KEY\n".  The second format is "enc(pub, aes_key)\n<more content>".

  // We read the first 4 bytes, see if it's the first format, if so handle it,
  // and if not read another 253 bytes to see if it's the second format.

  unsigned char line1[LINE1_FMT2_LEN + 1] = {0};
  buf_t line1a(LINE1_FMT1_LEN, line1);
  int amnt = reliable_get_to_eof_or_n(sd, line1a);
  // cout << "amnt: " << amnt << endl;

  // The only error we can detect is premature connection close:
  if (amnt < LINE1_FMT1_LEN) {
    std::cerr << "Unable to read " << (LINE1_FMT1_LEN + 1) << " bytes\n";
    // NB: can't even send a good error, because the connection is closed
    return true;
  }

  // detect and handle commands from the first format:
  else if (amnt == LINE1_FMT1_LEN &&
      strncmp((char *)line1, REQ_BYE.c_str(), REQ_BYE.length()) == 0 &&
      line1[REQ_KEY.length()] == '\n') {
    cout << "bye" << endl;
    send_reliably(sd, buf_t(RES_OK));
    return true;
  }
  else if (amnt == LINE1_FMT1_LEN &&
      strncmp((char *)line1, REQ_KEY.c_str(), REQ_KEY.length()) == 0 &&
      line1[REQ_KEY.length()] == '\n') {
    cout << "key" << endl;
    server_cmd_key(sd, pub);
    return false;
  }
  else{
    buf_t line1b(LINE1_FMT2_LEN - LINE1_FMT1_LEN, line1 + LINE1_FMT1_LEN);
    int amnt2 = reliable_get_to_eof_or_n(sd, line1b);
    // cout << "amnt2: " << amnt2 << endl;
    reliable_get_to_eof_or_n(sd, buf_t(1));

    //decrypt line1
    buf_t line1_buf(LINE1_FMT2_LEN, line1);
    buf_t aes_key = decrypted_rsakey(pri, line1_buf.bytes);

    //aes decrypt context
    EVP_CIPHER_CTX * aes_context = create_aes_context(aes_key.bytes, false);

    buf_t line2(LINE2_LEN);
    int amnt3 = reliable_get_to_eof_or_n(sd, line2);
    // cout << "amnt3: " << amnt3 << endl; 
    string length_str = string((char*)line2.bytes,line2.num);
    int length = stoi(length_str);
    reliable_get_to_eof_or_n(sd, buf_t(1));

    //decrypt line3
    buf_t msg_buf(length);
    int amnt4 = reliable_get_to_eof_or_n(sd, msg_buf);
    // cout << "amnt4: " << amnt4 << endl; 
    buf_t decrypted_line3 = aes_crypt_msg(aes_context, msg_buf);
    string command;
    istringstream line3_sstream(string((char *)decrypted_line3.bytes, decrypted_line3.num));
    getline(line3_sstream, command);
    cout << "command: " << command << endl;

    reset_aes_context(aes_context, aes_key.bytes, true);

    //reg
    if(strncmp(command.c_str(), REQ_REG.c_str(), REQ_REG.length()) == 0){
      server_cmd_reg(sd, dir, aes_context, decrypted_line3);
    }

    //set
    else if(strncmp(command.c_str(), REQ_SET.c_str(), REQ_SET.length()) == 0){
      server_cmd_set(sd, dir, aes_context, decrypted_line3);
    }

    //get
    else if(strncmp(command.c_str(), REQ_GET.c_str(), REQ_GET.length()) == 0){
      server_cmd_get(sd, dir, aes_context, decrypted_line3);
    }

    //all
    else if(strncmp(command.c_str(), REQ_ALL.c_str(), REQ_ALL.length()) == 0){
      server_cmd_all(sd, dir, aes_context, decrypted_line3);
    }

    else
      return true;

    free(aes_key.bytes);
    free(line2.bytes);
    free(decrypted_line3.bytes);

    return false;
  }

  return true;
}