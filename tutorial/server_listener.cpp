int main(int argc, char *argv[]) {
  // parse the command line arguments
  arg_t args;
  parse_args(argc, argv, args);
  if (args.usage) {
    usage(argv[0]);
    exit(0);
  }

  // Set up the server socket for listening.  This will exit the program on any
  // error.
  int serverSd = create_server_socket(args.port);

  // Use accept() to wait for a client to connect.  When it connects, service
  // it.  When it disconnects, then and only then will we accept a new client.
  while (true) {
    printf("Waiting for a client to connect...\n");
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int connSd = accept(serverSd, (sockaddr *)&clientAddr, &clientAddrSize);
    if (connSd < 0) {
      close(serverSd);
      error_message_and_exit(0, errno, "Error accepting request from client: ");
    }
    char clientname[1024];
    printf("Connected to %s\n", inet_ntop(AF_INET, &clientAddr.sin_addr,
                                          clientname, sizeof(clientname)));
    echo_server(connSd, true);
    // NB: ignore errors in close()
    close(connSd);
  }
  // NB: unreachable, but if we had a way to gracefully shut down, we'd need to
  //     close the server socket like this:
  close(serverSd);
  return 0;
}