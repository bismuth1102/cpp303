// [303] This file should not require any modifications for assignment #1

// [303] For this assignment, our focus is on getting some basic security stuff
//       to work correctly when we are transmitting data over the network, and
//       to become more familiar with techniques for persisting data.  To wrap
//       those goals in something realistic, we will create a secure company
//       user directory.

// [303] Be sure to read protocol.h for information about the messages that can
//       be sent between the client and the server.  These should be enough for
//       you to infer the behavior of the system.

// [303] Your programs should never require keyboard input. In particular, the
//       client should get all its parameters from the command line.

// [303] Your server should serialize its data to a file, so that it can be
//       stopped and restarted without losing any data.  It is sufficient for
//       your server to save everything to /one/ file.

// [303] Your server should *not* store plain-text passwords in the file.

// [303] Your code will probably be much simpler if you use C++ well.

// [303] Note that we are providing *a lot* of code to help you get started. You
//       should try to understand everything that is happening in the code we
//       provide, and also why this code might differ from the code in the
//       tutorials.  For example, the server should not exit on an error,
//       because that would make it possible for clients to cause service
//       disruptions just by being buggy.

// [303] As you read the code, comments marked with [303] will help you to know
//       specifics about the assignment (versus details about the code).  For
//       example, files that do not require any modifications will be marked at
//       the top, just like this file.

// [303] The basic_tests.sh script is a good way to test your code after you
//       think you have everything working.  It doesn't test for errors, but it
//       is a good start for making sure your implementations of the client and
//       server are correct.

#include <iostream>
#include <openssl/rsa.h>
#include <string.h>
#include <unistd.h>

#include "common_buf.h"
#include "common_crypto.h"
#include "common_file.h"
#include "common_net.h"
#include "server_args.h"
#include "server_parsing.h"
#include "server_storage.h"

int main(int argc, char **argv) {
  // Parse the command-line arguments
  arg_t args;
  parse_args(argc, argv, args);
  if (args.usage) {
    usage(argv[0]);
    return 0;
  }

  // print the configuration
  std::cout << "Listening on port " << args.port << " using (key/data) = ("
            << args.keyfile << ", " << args.datafile << ")\n";

  // If the key files don't exist, create them and then load the private key.
  RSA *pri = init_RSA(args.keyfile);
  if (pri == nullptr) {
    return -1;
  }

  // load the public key file contents
  buf_t pub = load_entire_file(args.keyfile + ".pub");
  if (pub.num == -1) {
    RSA_free(pri);
    return -1;
  }

  // If the data file exists, load the data into the directory.  Otherwise,
  // create an empty directory.
  Directory *dir = new Directory(args.datafile);
  dir->load();

  // Start listening for connections.
  int sd = create_server_socket(args.port);

  // On a connection, parse the message, then dispatch
  accept_client(sd, [&](int sd) { return serve_client(sd, pri, pub, dir); });

  // NB: unreachable, but if we had a way to gracefully shut down, we'd need to
  //     close the server socket like this:
  close(sd);

  // Reclaim memory
  delete dir;
  RSA_free(pri);
  free(pub.bytes);

  std::cerr << "Server terminated\n";
}