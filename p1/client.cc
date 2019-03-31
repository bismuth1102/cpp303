// [303] This file should not require any modifications for assignment #1

#include <openssl/rsa.h>
#include <unistd.h>

#include "client_args.h"
#include "client_commands.h"
#include "common_crypto.h"
#include "common_file.h"
#include "common_net.h"

int main(int argc, char **argv) {
  // Parse the command-line arguments
  arg_t args;
  parse_args(argc, argv, args);
  if (args.usage) {
    usage(argv[0]);
    return 0;
  }

  // If we don't have the keyfile on disk, get the file from server.  Once we
  // have the file, load the server's key.
  RSA *pubkey = nullptr;
  if (!file_exists(args.keyfile)) {
    int sd = connect_to_server(args.server, args.port);
    client_key(sd, args.keyfile);
    close(sd);
  }
  pubkey = load_pub(args.keyfile.c_str());

  // Connect to the server and perform the appropriate operation.  Note that we
  // don't have an option for sending KEY... it should happen in the background,
  // as needed
  int sd = connect_to_server(args.server, args.port);
  if (args.command == 'R') {
    client_register(sd, pubkey, args.username, args.userpass);
  } else if (args.command == 'B') {
    client_bye(sd);
  } else if (args.command == 'S') {
    client_set(sd, pubkey, args.username, args.userpass, args.setfile);
  } else if (args.command == 'G') {
    client_get(sd, pubkey, args.username, args.userpass, args.getuser);
  } else if (args.command == 'A') {
    client_all(sd, pubkey, args.username, args.userpass, args.allfile);
  }

  // Always clean up files and free memory when we are done
  RSA_free(pubkey);
  close(sd);
}