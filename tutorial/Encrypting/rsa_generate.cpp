/**
 * Print an error message and exit the program
 *
 * @param err The error code to return
 * @param msg The message to display
 */
void print_error_and_exit(int err, const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(err);
}

/**
 * Produce an RSA key and save its public and private parts to files
 *
 * @param pub The name of the public key file to generate
 * @param pri The name of the private key file to generate
 */
void generate_rsa_key_files(std::string pub, std::string pri) {
  printf("Generating RSA keys as (%s, %s)\n", pub.c_str(), pri.c_str());
  // When we create a new RSA keypar, we need to know the #bits (see constant
  // above) and the desired exponent to use in the public key.  The exponent
  // needs to be a bignum.  We'll use the RSA_F4 default value:
  BIGNUM *bn = BN_new();
  if (bn == nullptr) {
    print_error_and_exit(0, "Error in BN_set_word()");
  }
  if (BN_set_word(bn, RSA_F4) != 1) {
    BN_free(bn);
    print_error_and_exit(0, "Error in BN_set_word()");
  }

  // Now we can create the key pair
  RSA *rsa = RSA_new();
  if (rsa == nullptr) {
    BN_free(bn);
    print_error_and_exit(0, "Error in RSA_new()");
  }
  if (RSA_generate_key_ex(rsa, RSA_KEY_SIZE, bn, NULL) != 1) {
    BN_free(bn);
    RSA_free(rsa);
    print_error_and_exit(0, "Error in RSA_genreate_key_ex()");
  }

  // Create/truncate the files
  FILE *pubfile = fopen(pub.c_str(), "w");
  if (pubfile == nullptr) {
    BN_free(bn);
    RSA_free(rsa);
    perror("Error opening public key file for output");
    exit(0);
  }
  FILE *prifile = fopen(pri.c_str(), "w");
  if (prifile == nullptr) {
    BN_free(bn);
    RSA_free(rsa);
    fclose(pubfile);
    perror("Error opening private key file for output");
    exit(0);
  }

  // Perform the writes.  Defer cleanup on error, because the cleanup is the
  // same
  if (PEM_write_RSAPublicKey(pubfile, rsa) != 1) {
    fprintf(stderr, "Error writing public key\n");
  } else if (PEM_write_RSAPrivateKey(prifile, rsa, NULL, NULL, 0, NULL, NULL) !=
             1) {
    fprintf(stderr, "Error writing private key\n");
  } else {
    printf("Done\n");
  }

  // Cleanup regardless of whether the writes succeeded or failed
  fclose(pubfile);
  fclose(prifile);
  BN_free(bn);
  RSA_free(rsa);
}