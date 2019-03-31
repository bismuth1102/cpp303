// [303] This file should not require any modifications for assignment #1

#pragma once

#include <iostream>
#include <libgen.h>
#include <unistd.h>

/** arg_t is used to store the command-line arguments of the program */
struct arg_t {
  /** The port on which to listen */
  int port;

  /** The file for storing the company directory */
  std::string datafile;

  /** The file holding the RSA key */
  std::string keyfile;

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
  while ((opt = getopt(argc, argv, "p:f:k:h")) != -1) {
    switch (opt) {
    case 'p':
      args.port = atoi(optarg);
      break;
    case 'f':
      args.datafile = std::string(optarg);
      break;
    case 'k':
      args.keyfile = std::string(optarg);
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
      << basename(progname) << ": company user directory server\n"
      << "  -p [int]    Port on which to listen for incoming connections\n"
      << "  -f [string] File for storing the company directory\n"
      << "  -k [string] Basename of file for storing the server's RSA keys\n"
      << "  -h          Print help (this message)\n";
}