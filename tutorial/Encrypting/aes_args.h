/**
 * crypto_aes.cc
 *
 * Crypto_aes demonstrates how to use symmetric AES encryption/decryption on a
 * chunk of data of arbitrary size.
 *
 * Note that AES keys are usually ephemeral: we make them, share them, use them
 * briefly, and discard them.  This demo saves an AES key to a file so that we
 * can convince ourselves that it works, but in general, saving AES keys is a
 * bad practice.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string>
#include <unistd.h>
#include <libgen.h>


/**
 * Display a help message to explain how the command-line parameters for this
 * program work
 *
 * @progname The name of the program
 */
void usage(char *progname) {
  printf("%s: Perform basic AES encryption/decryption tasks.\n",
         basename(progname));
  printf("  -k [string] Name of the file holding the AES key\n");
  printf("  -i [string] Name of the input file to encrypt/decrypt\n");
  printf("  -o [string] Name of the output file to produce\n");
  printf("  -d          Decrypt from input to output using key\n");
  printf("  -e          Encrypt from input to output using key\n");
  printf("  -g          Generate a key file\n");
  printf("  -h       Print help (this message)\n");
}

/** arg_t is used to store the command-line arguments of the program */
struct arg_t {
  /** The file holding the AES key */
  std::string keyfile;

  /** The input file */
  std::string infile;

  /** The output file */
  std::string outfile;

  /** Should we decrypt? */
  bool decrypt = false;

  /** Should we encrypt? */
  bool encrypt = false;

  /** Should we generate a key? */
  bool generate = false;

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
  while ((opt = getopt(argc, argv, "k:i:o:degh")) != -1) {
    switch (opt) {
    case 'k':
      args.keyfile = std::string(optarg);
      break;
    case 'i':
      args.infile = std::string(optarg);
      break;
    case 'o':
      args.outfile = std::string(optarg);
      break;
    case 'd':
      args.decrypt = true;
      break;
    case 'e':
      args.encrypt = true;
      break;
    case 'g':
      args.generate = true;
      break;
    case 'h':
      args.usage = true;
      break;
    }
  }
}