#include <arpa/inet.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <netdb.h>
#include <string>
#include <unistd.h>

#include "common_buf.h"
#include "common_err.h"
#include "common_pool.h"

/**
 * Send a buffer of data over a socket.
 *
 * @param sd  The socket on which to send
 * @param msg The message to send
 *
 * @returns True if the whole buffer was sent, false otherwise
 */
bool send_reliably(int sd, buf_t msg) {
  // When we send, we need to be ready for the possibility that not all the
  // data will transmit at once
  const unsigned char *next_byte = msg.bytes;
  int remain = msg.num;
  while (remain) {
    int sent = send(sd, next_byte, remain, 0);
    // NB: Sending 0 bytes means the server closed the socket, and we should
    //     fail, so it's only EINTR that is recoverable.
    if (sent <= 0) {
      if (errno != EINTR) {
        sys_error(errno, "Error in send():");
        return false;
      }
    } else {
      next_byte += sent;
      remain -= sent;
    }
  }
  return true;
}

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
int reliable_get_to_eof_or_n(int sd, buf_t dest) {
  memset(dest.bytes, 0, dest.num);
  int remain = dest.num;
  unsigned char *next_byte = dest.bytes;
  int total = 0;
  while (remain) {
    int rcd = recv(sd, next_byte, remain, 0);
    // NB: 0 bytes received means server closed socket, and -1 means an error.
    //     EINTR means try again, otherwise we will just fail
    if (rcd <= 0) {
      if (errno != EINTR) {
        if (rcd == 0) {
          return total;
        } else {
          sys_error(errno, "Error in recv():");
          return -1;
        }
      }
    } else {
      next_byte += rcd;
      remain -= rcd;
      total += rcd;
    }
  }
  return total;
}

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
buf_t reliable_get_to_eof(int sd) {
  // set up the initial buffer
  buf_t res(16);
  int recd = 0;
  // start reading.  Double the buffer any time we fill up
  while (true) {
    int remain = res.num - recd;
    int justgot = recv(sd, (res.bytes + recd), remain, 0);
    // EOF means we're done reading
    if (justgot == 0) {
      res.num = recd;
      return res;
    }
    // On error, fail for non-EINTR, no-op on EINTR
    else if (justgot < 0) {
      if (errno != EINTR) {
        sys_error(errno, "Error in recv():");
        free(res.bytes);
        return {-1, nullptr};
      }
    }
    // bytes received.  advance pointer, maybe double the buffer
    else {
      recd += justgot;
      if (recd == res.num) {
        unsigned char *newbuf = (unsigned char *)malloc(2 * res.num);
        memset(newbuf, 0, 2 * res.num);
        memcpy(newbuf, res.bytes, res.num);
        free(res.bytes);
        res.bytes = newbuf;
        res.num = res.num * 2;
      }
    }
  }
}

/**
 * Connect to a server so that we can have bidirectional communication on the
 * socket (represented by a file descriptor) that this function returns
 *
 * @param hostname The name of the server (ip or DNS) to connect to
 * @param port     The server's port that we should use
 *
 * @returns The socket descriptor for further communication, or -1 on error
 */
int connect_to_server(std::string hostname, int port) {
  // figure out the IP address that we need to use and put it in a sockaddr_in
  struct hostent *host = gethostbyname(hostname.c_str());
  if (host == nullptr) {
    std::cerr << "connect_to_server():DNS error:" << hstrerror(h_errno)
              << std::endl;
    return -1;
  }
  sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr =
      inet_addr(inet_ntoa(*(struct in_addr *)*host->h_addr_list));
  addr.sin_port = htons(port);
  // create the socket and try to connect to it
  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    sys_error(errno, "Error making client socket: ");
    return -1;
  }
  if (connect(sd, (sockaddr *)&addr, sizeof(addr)) < 0) {
    close(sd);
    sys_error(errno, "Error connecting socket to address: ");
    return -1;
  }
  return sd;
}

/**
 * Create a server socket that we can use to listen for new incoming requests
 *
 * @param port The port on which the program should listen for new connections
 *
 * @returns The new listening socket, or -1 on error
 */
int create_server_socket(std::size_t port) {
  // A socket is just a kind of file descriptor.  We want our connections to use
  // IPV4 and TCP:
  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    sys_error(errno, "Error making server socket: ");
    return -1;
  }
  // The default is that when the server crashes, the socket can't be used for a
  // few minutes.  This lets us re-use the socket immediately:
  int tmp = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int)) < 0) {
    close(sd);
    sys_error(errno, "setsockopt(SO_REUSEADDR) failed: ");
    return -1;
  }

  // bind the socket to the server's address and the provided port, and then
  // start listening for connections
  sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(sd);
    sys_error(errno, "Error binding socket to local address: ");
    return -1;
  }
  if (listen(sd, 0) < 0) {
    close(sd);
    sys_error(errno, "Error listening on socket: ");
    return -1;
  }
  return sd;
}

/**
 * Given a listening socket, start calling accept() on it to get new
 * connections.  Each time a connection comes in, use the provided handler to
 * process the request.  Note that this is not multithreaded.  Only one client
 * will be served at a time.
 *
 * @param sd The socket file descriptor on which to call accept
 * @param handler A function to call when a new connection comes in
 */
void accept_client(int sd, std::function<bool(int)> handler) {
  // Use accept() to wait for a client to connect.  When it connects, service
  // it.  When it disconnects, then and only then will we accept a new client.
  while (true) {
    std::cout << "Waiting for a client to connect...\n";
    sockaddr_in clientAddr = {0};
    socklen_t clientAddrSize = sizeof(clientAddr);
    int connSd = accept(sd, (sockaddr *)&clientAddr, &clientAddrSize);
    if (connSd < 0) {
      close(sd);
      sys_error(errno, "Error accepting request from client: ");
      return;
    }
    char clientname[1024];
    std::cout << "Connected to "
              << inet_ntop(AF_INET, &clientAddr.sin_addr, clientname,
                           sizeof(clientname))
              << std::endl;
    bool done = handler(connSd);
    // NB: ignore errors in close()
    close(connSd);
    if (done)
      return;
  }
}

/**
 * Given a listening socket, start calling accept() on it to get new
 * connections.  Each time a connection comes in, pass it to the thread pool so
 * that it can be processed.
 *
 * @param sd   The socket file descriptor on which to call accept
 * @param pool The thread pool that handles new requests
 */
void accept_client(int sd, thread_pool &pool) {
  std::atomic<bool> safe_shutdown(false);
  pool.set_shutdown_handler([&]() {
    safe_shutdown = true;
    close(sd);
  });
  // Use accept() to wait for a client to connect.  When it connects, service
  // it.  When it disconnects, then and only then will we accept a new client.
  while (pool.check_active()) {
    std::cout << "Waiting for a client to connect...\n";
    sockaddr_in clientAddr = {0};
    socklen_t clientAddrSize = sizeof(clientAddr);
    int connSd = accept(sd, (sockaddr *)&clientAddr, &clientAddrSize);
    if (connSd < 0) {
      close(sd);
      // If safe_shutdown() was called, and it's EBADF, then the pool has been
      // halted, and the listening socket closed, so don't print an error.
      if (errno != EBADF || !safe_shutdown)
        sys_error(errno, "Error accepting request from client: ");
      return;
    }
    char clientname[1024];
    std::cout << "Connected to "
              << inet_ntop(AF_INET, &clientAddr.sin_addr, clientname,
                           sizeof(clientname))
              << std::endl;
    pool.service_connection(connSd);
  }
}