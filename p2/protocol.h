// [303] This file should not require any modifications for assignment #1

// [303] This file has been modified for assignment #2.  There are new error
//       messages and new command types (beginning on line 114).  Also, the file
//       was restructured to make it easier to understand.

// [303] This file should not require any modifications for assignment #2

#pragma once

#include <string>

// Here are the messages that a client may send, and the responses a server will
// provide.  Note that the entire request is a single byte stream, as is the
// entire response.  Different fields of a request may be encrypted in different
// ways, as indicated by 'enc()'.  Whitespace (spaces, tabs, newlines) in a
// description of a request or response. should be ignored.  When there is a
// 'len' field, it is the length of the remainder of the message, excluding its
// immediately following newline, and it obviates a final newline.  'enc(x,y)'
// indicates that y should be encrypted using key x.  aes_key should be
// generated for each request.

// Summary:    Request of server's public key, to use for subsequent interaction
//             with the server by the client.
// Request:    KEY\n
// Response:   public_key<EOF>
// Errors:     None
const std::string REQ_KEY = "KEY";

// Summary:    Force the server to stop.  Note that a real server should never
//             let a client cause it to stop.  This is a convenience method to
//             help the professor and TAs grade your assignment.
// Request:    BYE\n
// Response:   OK<EOF>
// Errors:     None
const std::string REQ_BYE = "BYE";

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
const std::string REQ_REG = "REG";

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
const std::string REQ_SET = "SET";

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
const std::string REQ_GET = "GET";

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
const std::string REQ_ALL = "ALL";

// Purpose: Indicate the request was successful
const std::string RES_OK = "OK";

// Purpose: Indicate the request involved a nonexistent user
const std::string RES_ERR_USER_EXISTS = "ERR_USER_EXISTS";

// Purpose: Indicate the request led to a cryptography error
const std::string RES_ERR_CRYPTO = "ERR_CRYPTO";

// Purpose: Indicate the request was malformed
const std::string RES_ERR_MESSAGE_FORMAT = "ERR_MESSAGE_FORMAT";

// Purpose: Indicate the request had a bad password
const std::string RES_ERR_LOGIN = "ERR_LOGIN";

// Purpose: Indicate the request had an invalid command
const std::string RES_ERR_INV_CMD = "ERR_INVALID_COMMAND";

// Purpose: Indicate the request was for something that returns no data
const std::string RES_ERR_NO_DATA = "ERR_NO_DATA";

// Purpose: Indicate a network transmission error
const std::string RES_ERR_XMIT = "ERR_XMIT";

// [303] Below are the additions for assignment #2

// Purpose:    Insert a new key/value pair
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, KEYINS\nuser_name\npassword\nkey\nnum_bytes\nval)
// Response:   enc(aes_key, OK)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_LOGIN, ERR_CRYPTO, ERR_MESSAGE_FORMAT, ERR_BADKEY
// Notes:      each num_bytes is the number of bytes in the subsequent part of
//             the message.  Note that these parts will not be newline
//             delimited.  There will not be a newline immediately before the
//             <EOF> in the response.
const std::string REQ_KEYINS = "KEYINS";

// Purpose:    Get the value for some key
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, KEYGET\nuser_name\npassword\nkey)
// Response:   enc(aes_key, OK\nnum_bytes\nval)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_LOGIN, ERR_CRYPTO, ERR_MESSAGE_FORMAT, ERR_BADKEY
// Notes:      See KEYINS for information about the request format.  There will
//             not be a newline immediately before the <EOF> in the response.
const std::string REQ_KEYGET = "KEYGET";

// Purpose:    Delete a key/value mapping
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, KEYDEL\nuser_name\npassword\nkey)
// Response:   enc(aes_key, OK)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_LOGIN, ERR_CRYPTO, ERR_MESSAGE_FORMAT, ERR_BADKEY
// Notes:      See KEYINS for information about the request format.  There will
//             not be a newline immediately before the <EOF> in the response.
const std::string REQ_KEYDEL = "KEYDEL";

// Purpose:    Update a key/value mapping
// Request:    enc(public_key, aes_key)\n
//             enc(aes_key, len)\n
//             enc(aes_key, KEYUPD\nuser_name\npassword\nkey\nnum_bytes\nval)
// Response:   enc(aes_key, OK)<EOF>
//               *or*
//             enc(aes_key, error_code)<EOF>
// Errors:     ERR_LOGIN, ERR_CRYPTO, ERR_MESSAGE_FORMAT, ERR_BADKEY
// Notes:      See KEYINS for information about the request format.  There will
//             not be a newline immediately before the <EOF> in the response.
const std::string REQ_KEYUPD = "KEYUPD";

// Summary:    Force the server to save its directory.  Note that a real server
//             should never let a client decide when data should persist.  This
//             is a temporary solution to persistence, which we will replace in
//             the next assignment.
// Request:    SAV\n
// Response:   OK<EOF>
// Errors:     None
const std::string REQ_SAVE = "SAV";

// Purpose: Indicate that the request entailed an invalid key
const std::string RES_ERR_BADKEY = "ERR_BADKEY";