#include <iostream>
#include "aes.h"
#include "aes_args.h"

int main(int argc, char **argv) {
  // Parse the command-line arguments
  arg_t args;
  parse_args(argc, argv, args);
  if (args.usage) {
    usage(argv[0]);
    return 0;
  }

  // If the user requested the creation of an AES key, just make the key and
  // exit
  if (args.generate) {
    generate_aes_key_file(args.keyfile);
    return 0;
  }

  // Get the AES key from the file, and make an AES encryption context suitable
  // for either encryption or decryption
  EVP_CIPHER_CTX *ctx = get_aes_context(args.keyfile, args.encrypt);

  // Open the input and output files... Output file gets truncated
  FILE *infile = fopen(args.infile.c_str(), "rb");
  if (!infile) {
    perror("Error opening input file");
    EVP_CIPHER_CTX_cleanup(ctx);
    exit(0);
  }
  FILE *outfile = fopen(args.outfile.c_str(), "wb");
  if (!outfile) {
    perror("Error opening output file");
    EVP_CIPHER_CTX_cleanup(ctx);
    exit(0);
  }

  // Do the encryption or decryption.  Since it's symmetric, the call is the
  // same :)
  bool res = aes_crypt(ctx, infile, outfile);
  if (!res) {
    fprintf(stderr, "Error calling aes_crypt()");
  }
  fclose(infile);
  fclose(outfile);
  EVP_CIPHER_CTX_cleanup(ctx);
}