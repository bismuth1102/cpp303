// [303] This file should not require any modifications for assignment #1

#pragma once

#include <string>

// Here are the messages that a client may send, and the responses a server will
// provide.  Note that the entire request is a single byte stream, as is the
// entire response.  Different fields of a request may be encrypted in different
// ways, as indicated by 'enc()'.  Whitespace (spaces, tabs, newlines) in a
// description of a request or response should be ignored.  When there is a
// 'len' field, it is the length of the remainder of the message, excluding its
// immediately following newline, and it obviates a final newline.  'enc(x,y)'
// indicates that y should be encrypted using key x.  aes_key should be
// generated for each request.

// Summary:    Request of server's public key, to use for subsequent interaction
//             with the server by the client.
// Request:    KEY\n
// Response:   public_key<EOF>
// Errors:     None

// Summary:    Force the server to stop.  Note that a real server should never
//             let a client cause it to stop.  This is a convenience method to
//             help the professor and TAs grade your assignment.
// Request:    BYE\n
// Response:   OK<EOF>
// Errors:     None

// Purpose:    Creates a new user in the directory, with null content.
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, REG\nuser_name\npassword)
// Response:   enc(aes_key,OK)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_USER_EXISTS, ERR_CRYPTO, ERR_MESSAGE_FORMAT
// Notes:      A user name must be unique in the directory, and must be no more
//             than 64 characters.  A password must be no more than 128
//             characters.

// Purpose:    Allow a user to set her/his user-controlled content in the
//             company directory.
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, SET\nuser_name\npassword\nnum_bytes\nbytes)
// Response:   enc(aes_key,OK)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_LOGIN, ERR_CRYPTO, ERR_MESSAGE_FORMAT
// Notes:      The maximum number of bytes is 2^20.  Bytes may be ASCII or
//             binary.

// Purpose:    Get the content for a user in the directory.
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, GET\nuser_name\npassword\nuser_name_to_get)
// Response:   enc(aes_key, OK\nnum_bytes\nbytes)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_LOGIN, ERR_CRYPTO, ERR_MESSAGE_FORMAT, ERR_NO_USER,
//             ERR_NO_DATA
// Notes:      The maximum number of bytes is 2^20.  Bytes may be ASCII or
//             binary.

// Purpose:    Get the listing of all users
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, ALL\nuser_name\npassword)
// Response:   enc(aes_key, OK\nnum_bytes\nuser_0\nuser_1\nuser_2...)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_LOGIN, ERR_CRYPTO, ERR_MESSAGE_FORMAT
// Notes:      num_bytes is the number of bytes in the return message.  The
//             current user's name will always be part of the list.  There will
//             not be a newline immediately before the <EOF> in the response.

// Below is a set of "magic constants" to use instead of manually typing the
// strings that will be used by the protocol.

const std::string REQ_KEY = "KEY";
const std::string REQ_BYE = "BYE";
const std::string REQ_REG = "REG";
const std::string REQ_SET = "SET";
const std::string REQ_GET = "GET";
const std::string REQ_ALL = "ALL";
const std::string RES_OK = "OK";
const std::string RES_ERR_USER_EXISTS = "ERR_USER_EXISTS";
const std::string RES_ERR_CRYPTO = "ERR_CRYPTO";
const std::string RES_ERR_MESSAGE_FORMAT = "ERR_MESSAGE_FORMAT";
const std::string RES_ERR_LOGIN = "ERR_LOGIN";
const std::string RES_ERR_INV_CMD = "ERR_INVALID_COMMAND";
const std::string RES_ERR_NO_DATA = "ERR_NO_DATA";
const std::string RES_ERR_XMIT = "ERR_XMIT";