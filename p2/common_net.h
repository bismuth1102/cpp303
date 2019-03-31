#pragma once

#include <functional>
#include <string>

#include "common_buf.h"
#include "common_pool.h"

/**
 * Send a buffer of data over a socket.
 *
 * @param sd  The socket on which to send
 * @param msg The message to send
 *
 * @returns True if the whole buffer was sent, false otherwise
 */
bool send_reliably(int sd, buf_t msg);
/**
 * Perform a reliable read when we have a guess about how many bytes we might
 * get, but it's OK if the socket EOFs before we get that many bytes.
 *
 * @param sd     The socket from which to read
 * @param buffer The buffer into which the data should go.  It is assumed to be
 *               pre-allocated to max bytes.
 * @param max    The maximum number of bytes to get
 *
 * @returns The actual number of bytes read, or -1 on a non-eof error
 */
int reliable_get_to_eof_or_n(int sd, buf_t dest);

/**
 * Perform a reliable read when we are not sure how many bytes we are going to
 * receive.
 *
 * @param sd     The socket from which to read
 * @param buffer The buffer into which the data should go.  It is assumed to be
 *               pre-allocated to max bytes.
 * @param max    The maximum number of bytes to get
 *
 * @returns A buf_t with the data that was read, or a buf_t with -1 on error
 */
buf_t reliable_get_to_eof(int sd);

/**
 * Connect to a server so that we can have bidirectional communication on the
 * socket (represented by a file descriptor) that this function returns
 *
 * @param hostname The name of the server (ip or DNS) to connect to
 * @param port     The server's port that we should use
 *
 * @returns The socket descriptor for further communication, or -1 on error
 */
int connect_to_server(std::string hostname, int port);

/**
 * Create a server socket that we can use to listen for new incoming requests
 *
 * @param port The port on which the program should listen for new connections
 *
 * @returns The new listening socket, or -1 on error
 */
int create_server_socket(std::size_t port);

/**
 * Given a listening socket, start calling accept() on it to get new
 * connections.  Each time a connection comes in, use the provided handler to
 * process the request.  Note that this is not multithreaded.  Only one client
 * will be served at a time.
 *
 * @param sd The socket file descriptor on which to call accept
 * @param handler A function to call when a new connection comes in
 */
void accept_client(int sd, std::function<bool(int)> handler);

/**
 * Given a listening socket, start calling accept() on it to get new
 * connections.  Each time a connection comes in, pass it to the thread pool so
 * that it can be processed.
 *
 * @param sd   The socket file descriptor on which to call accept
 * @param pool The thread pool that handles new requests
 */
void accept_client(int sd, thread_pool &pool);