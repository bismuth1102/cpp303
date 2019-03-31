/**
 * Create a server socket that we can use to listen for new incoming requests
 *
 * @param port The port on which the program should listen for new connections
 */
int create_server_socket(std::size_t port) {
  // A socket is just a kind of file descriptor.  We want our connections to use
  // IPV4 and TCP:
  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    error_message_and_exit(0, errno, "Error making server socket: ");
  }
  // The default is that when the server crashes, the socket can't be used for a
  // few minutes.  This lets us re-use the socket immediately:
  int tmp = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int)) < 0) {
    close(sd);
    error_message_and_exit(0, errno, "setsockopt(SO_REUSEADDR) failed: ");
  }

  // bind the socket to the server's address and the provided port, and then
  // start listening for connections
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(sd);
    error_message_and_exit(0, errno, "Error binding socket to local address: ");
  }
  if (listen(sd, 0) < 0) {
    close(sd);
    error_message_and_exit(0, errno, "Error listening on socket: ");
  }
  return sd;
}