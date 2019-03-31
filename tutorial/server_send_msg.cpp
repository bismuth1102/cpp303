/**
 * Receive text over the provided socket file descriptor, and send it back to
 * the client.  When the client sends an EOF, return.
 *
 * @param sd      The socket file descriptor to use for the echo operation
 * @param verbose Should stats be printed upon completion?
 */
void echo_server(int sd, bool verbose) {
  // vars for tracking connection duration, bytes transmitted
  size_t xmitBytes = 0;
  struct timeval start_time, end_time;
  if (verbose) {
    gettimeofday(&start_time, nullptr);
  }

  // read data for as long as there is data, and always send it back
  while (true) {
    // Receive up to 16 bytes of data
    //
    // NB: we should receive more data at a time, but we'll keep it small so
    //     that it's easier to see how the server handles full buffers
    char buf[16] = {0};
    // NB: see text_client for explanation of why we receive data like this
    ssize_t rcd = recv(sd, buf, sizeof(buf), 0);
    if (rcd <= 0) {
      if (errno != EINTR) {
        if (rcd == 0) {
          break;
        } else {
          // NB: throughout this function, we are crashing if the client does
          //     something bad.  That's not a good way to write reliable code.
          //     Instead, this function should just print an error and return,
          //     so that it can start servicing another client.
          error_message_and_exit(0, errno, "Error in recv(): ");
        }
      }
    } else {
      // Immediately send back whatever we got
      //
      // NB: see text_client for explanation of why we send data like this
      xmitBytes += rcd;
      char *next_byte = buf;
      std::size_t remain = rcd;
      while (remain) {
        std::size_t sent = send(sd, next_byte, remain, 0);
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
      xmitBytes += rcd;
    }
  }
  if (verbose) {
    gettimeofday(&end_time, nullptr);
    printf("Transmitted %ld bytes in %ld seconds\n", xmitBytes,
           (end_time.tv_sec - start_time.tv_sec));
  }
}