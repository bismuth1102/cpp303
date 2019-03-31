// [303] This file has been updated for assignment #2

// [303] This file was modified between assignments #1 and #2.  There are new
//       command-line options.

// [303] This file should not require any modifications for assignment #2

#pragma once

#include <iostream>
#include <libgen.h>
#include <unistd.h>

/** arg_t is used to store the command-line arguments of the program */
struct arg_t {
  /** The port on which to listen */
  int port = 0;

  /** The IP or hostname of the server */
  std::string server = "";

  /** The file for storing the server's public key */
  std::string keyfile = "";

  /** The user's name */
  std::string username = "";

  /** The user's password */
  std::string userpass = "";

  /** The command to execute (B/R/S/G/A) */
  char command = '-';

  /** If the command is S, this is the file to send */
  std::string setfile = "";

  /** If the command is G, this is the user to look up */
  std::string getuser = "";

  /** If the command is A, this is the file for saving results */
  std::string allfile = "";

  /** For kvstore commands, this is the key */
  std::string key = "";

  /** for kvstore commands, this is the file with the value */
  std::string vfile = "";

  /** Display a usage message? */
  bool usage = false;
};

/**
 * Parse the command-line arguments, and use them to populate the provided args
 * object.
 *
 * @param argc The number of command-line arguments passed to the program
 * @param argv The list of command-line arguments
 * @param args The struct into which the parsed args should go
 */
void parse_args(int argc, char **argv, arg_t &args) {
  long opt;
  while ((opt = getopt(argc, argv, "k:u:w:s:p:RS:G:A:BI:U:D:T:Vv:h")) != -1) {
    switch (opt) {
    case 'p':
      args.port = atoi(optarg);
      break;
    case 's':
      args.server = std::string(optarg);
      break;
    case 'k':
      args.keyfile = std::string(optarg);
      break;
    case 'u':
      args.username = std::string(optarg);
      break;
    case 'w':
      args.userpass = std::string(optarg);
      break;
    case 'B':
      args.command = (args.command == '-') ? 'B' : '!';
      args.usage |= args.command == '!';
      break;
    case 'R':
      args.command = (args.command == '-') ? 'R' : '!';
      args.usage |= args.command == '!';
      break;
    case 'S':
      args.command = (args.command == '-') ? 'S' : '!';
      args.usage |= args.command == '!';
      args.setfile = std::string(optarg);
      break;
    case 'G':
      args.command = (args.command == '-') ? 'G' : '!';
      args.usage |= args.command == '!';
      args.getuser = std::string(optarg);
      break;
    case 'A':
      args.command = (args.command == '-') ? 'A' : '!';
      args.usage |= args.command == '!';
      args.allfile = std::string(optarg);
      break;
    case 'h':
      args.usage = true;
      break;
    case 'I':
      args.command = (args.command == '-') ? 'I' : '!';
      args.usage |= args.command == '!';
      args.key = std::string(optarg);
      break;
    case 'U':
      args.command = (args.command == '-') ? 'U' : '!';
      args.usage |= args.command == '!';
      args.key = std::string(optarg);
      break;
    case 'D':
      args.command = (args.command == '-') ? 'D' : '!';
      args.usage |= args.command == '!';
      args.key = std::string(optarg);
      break;
    case 'T':
      args.command = (args.command == '-') ? 'T' : '!';
      args.usage |= args.command == '!';
      args.key = std::string(optarg);
      break;
    case 'V':
      args.command = (args.command == '-') ? 'V' : '!';
      args.usage |= args.command == '!';
      break;
    case 'v':
      args.vfile = std::string(optarg);
      break;
    default:
      args.usage = true;
      return;
    }
  }
}

/**
 * Display a help message to explain how the command-line parameters for this
 * program work
 *
 * @progname The name of the program
 */
void usage(char *progname) {
  std::cout
      << basename(progname) << ": company user directory client\n"
      << " Required Configuration Parameters:\n"
      << "  -k [file]   The filename for storing the server's public key\n"
      << "  -u [string] The username to use for authentication\n"
      << "  -w [string] The password to use for authentication\n"
      << "  -s [string] IP address or hostname of server\n"
      << "  -p [int]    Port to use to connect to server\n"
      << "  -v [file]   The file storing the value (for kvstore commands)\n"
      << " Commands (must provide exactly one):\n"
      << "  -R          Register a new user\n"
      << "  -S [file]   Set the user's data to the contents of the file\n"
      << "  -G [string] Get the user data for the provided username\n"
      << "  -A [file]   Get a list of all users' names, and save to a file\n"
      << "  -B          Force the server to stop\n"
      << "  -I [string] Perform a kvstore insert with given key (requires -v)\n"
      << "  -U [string] Perform a kvstore update with given key (requires -v)\n"
      << "  -D [string] Perform a kvstore delete with given key\n"
      << "  -T [string] Perform a kvstore get with given key (requires -v)\n"
      << "  -V          Send the server a 'save' message\n"
      << " Other Options:\n"
      << "  -h          Print help (this message)\n";
}