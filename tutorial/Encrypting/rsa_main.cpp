int main(int argc, char *argv[]) {
  // Parse the command-line arguments
  arg_t args;
  parse_args(argc, argv, args);
  if (args.usage) {
    usage(argv[0]);
    return 0;
  }

  if (args.generate) {
    generate_rsa_key_files(args.pubkeyfile, args.prikeyfile);
    return 0;
  }

  // Open the input and output files... Output file gets truncated
  FILE *infile = fopen(args.infile.c_str(), "rb");
  if (!infile) {
    perror("Error opening input file");
    exit(0);
  }
  FILE *outfile = fopen(args.outfile.c_str(), "wb");
  if (!outfile) {
    perror("Error opening output file");
    exit(0);
  }

  // Encrypt or decrypt, and clean up
  if (args.encrypt) {
    printf("Encrypting %s to %s\n", args.infile.c_str(), args.outfile.c_str());
    RSA *pub = load_pub(args.pubkeyfile.c_str());
    if (rsa_encrypt(pub, infile, outfile)) {
      printf("Success!\n");
    }
    RSA_free(pub);
  } else if (args.decrypt) {
    printf("Decrypting %s to %s\n", args.infile.c_str(), args.outfile.c_str());
    RSA *pri = load_pri(args.prikeyfile.c_str());
    if (rsa_decrypt(pri, infile, outfile)) {
      printf("Success!\n");
    }
    RSA_free(pri);
  }
  fclose(infile);
  fclose(outfile);
}