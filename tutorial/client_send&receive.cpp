/**
 * Receive text from the keyboard (well, actually, stdin), send it to the
 * server, and then print whatever the server sends back.
 *
 * @param sd      The socket file descriptor to use for the echo operation
 * @param verbose Should stats be printed upon completion?
 */
void echo_client(int sd, bool verbose) {
  // vars for tracking connection duration, bytes transmitted
  size_t xmitBytes = 0;
  struct timeval start_time, end_time;
  if (verbose) {
    gettimeofday(&start_time, nullptr);
  }

  // string for holding user input that we send to the server
  std::string data;

  // read from stdin for as long as it isn't EOF, send to server, print reply
  while (true) {
    // Get the data.  We are using C++ streams (cin) instead of scanf, because
    // it's easier and more portable than doing it in C
    //
    // NB: this assumes that stdin hasn't been redirected from a socket
    printf("Client: ");
    getline(std::cin, data);
    if (std::cin.eof()) {
      break;
    }

    // When we send, we need to be ready for the possibility that not all the
    // data will transmit at once
    //
    // NB: it's usually *very bad* to save a pointer to the inside of a C++
    //     string, and it's especially bad to have a non-const pointer.  But in
    //     this code, it's OK.
    char *next_byte = (char *)data.c_str();
    std::size_t remain = data.length();
    while (remain) {
      // NB: send() with last parameter 0 is the same as write() syscall
      std::size_t sent = send(sd, next_byte, remain, 0);
      // NB: Sending 0 bytes means the server closed the socket, and we should
      //     crash.
      //
      // NB: Sending -1 bytes means an error.  If the error is EINTR, it's OK,
      //     try again.  Otherwise crash.
      if (sent <= 0) {
        if (errno != EINTR) {
          error_message_and_exit(0, errno, "Error in send(): ");
        }
      } else {
        next_byte += sent;
        remain -= sent;
      }
    }
    // update the transmission count
    xmitBytes += data.length();

    // Now it's time to receive data.
    //
    // Receiving is hard when we don't know how much data we are going to
    // receive.  Two workarounds are (1) receive until a certain token comes in
    // (such as newline), or (2) receive a fixed number of bytes.  Since we're
    // expecting back exactly what we sent, we can take strategy #2.
    //
    // NB: need an extra byte in the buffer, so we can null-terminate the string
    //     before printing it.
    char buf[data.length() + 1] = {0};
    remain = data.length();
    next_byte = buf;
    while (remain) {
      // NB: recv() with last parameter 0 is the same as read() syscall
      ssize_t rcd = recv(sd, next_byte, remain, 0);
      // NB: as above, 0 bytes received means server closed socket, and -1 means
      //     an error.  EINTR means try again, otherwise we will just crash.
      if (rcd <= 0) {
        if (errno != EINTR) {
          if (rcd == 0) {
            fprintf(stderr, "Error in recv(): EOF\n");
            exit(0);
          } else {
            error_message_and_exit(0, errno, "Error in recv(): ");
          }
        }
      } else {
        next_byte += rcd;
        remain -= rcd;
      }
    }
    // Print back the message from the server, and update the transmission count
    xmitBytes += data.length();
    printf("Server: %s\n", buf);
  }
  if (verbose) {
    gettimeofday(&end_time, nullptr);
    printf("Transmitted %ld bytes in %ld seconds\n", xmitBytes,
           (end_time.tv_sec - start_time.tv_sec));
  }
}