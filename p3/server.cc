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

// [303] This file should not require any modifications for assignment #1

// [303] This file was modified between assignments #1 and #2.  It now uses a
//       thread pool, and a different version of accept_client.

// [303] This file should not require any modifications for assignment #2

#include <iostream>
#include <openssl/rsa.h>
#include <string.h>
#include <unistd.h>

#include "common_buf.h"
#include "common_crypto.h"
#include "common_file.h"
#include "common_net.h"
#include "common_pool.h"
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

  // Create a thread pool that will invoke serve_client (from a pool thread)
  // each time a new socket is given to it.
  thread_pool pool(args.pool_size,
                   [&](int sd) { return serve_client(sd, pri, pub, dir); });

  // Start accepting connections and passing them to the pool.
  accept_client(sd, pool);

  // The program can't exit until all threads in the pool are done.
  pool.await_shutdown();

  close(sd);

  // Reclaim memory
  delete dir;
  RSA_free(pri);
  free(pub.bytes);

  std::cerr << "Server terminated\n";
}