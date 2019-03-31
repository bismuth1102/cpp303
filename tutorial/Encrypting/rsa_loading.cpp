/**
 * Load an RSA public key from the given filename
 *
 * @param filename The name of the file that has the public key in it
 */
RSA *load_pub(const char *filename) {
  FILE *pub = fopen(filename, "r");
  if (pub == nullptr) {
    perror("Error opening public key file");
    exit(0);
  }
  RSA *rsa = PEM_read_RSAPublicKey(pub, NULL, NULL, NULL);
  if (rsa == nullptr) {
    print_error_and_exit(0, "Error reading public key file");
  }
  return rsa;
}

/**
 * Load an RSA private key from the given filename
 *
 * @param filename The name of the file that has the private key in it
 */
RSA *load_pri(const char *filename) {
  FILE *pri = fopen(filename, "r");
  if (pri == nullptr) {
    perror("Error opening private key file");
    exit(0);
  }
  RSA *rsa = PEM_read_RSAPrivateKey(pri, NULL, NULL, NULL);
  if (rsa == nullptr) {
    print_error_and_exit(0, "Error reading public key file");
  }
  return rsa;
}