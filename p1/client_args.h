// [303] This file should not require any modifications for assignment #1

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
  while ((opt = getopt(argc, argv, "k:u:w:s:p:RS:G:A:Bh")) != -1) {
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
      << " Commands (must provide exactly one):\n"
      << "  -R          Register a new user\n"
      << "  -S [file]   Set the user's data to the contents of the file\n"
      << "  -G [string] Get the user data for the provided username\n"
      << "  -A [file]   Get a list of all users' names, and save to a file\n"
      << "  -B          Force the server to stop\n"
      << " Other Options:\n"
      << "  -h          Print help (this message)\n";
}