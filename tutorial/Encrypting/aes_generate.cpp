/**
 * Produce an AES key and save it to a file
 *
 * @param keyfile The name of the file to create
 */
void generate_aes_key_file(std::string keyfile) {
  // Generate an encryption key and initialization vector for AES, using a
  // cryptographically sound algorithm
  unsigned char key[AES_256_KEY_SIZE], iv[BLOCK_SIZE];
  if (!RAND_bytes(key, sizeof(key)) || !RAND_bytes(iv, sizeof(iv))) {
    fprintf(stderr, "Error in RAND_bytes()\n");
    exit(0);
  }

  // Create or truncate the key file
  FILE *file = fopen(keyfile.c_str(), "wb");
  if (!file) {
    perror("Error in fopen");
    exit(0);
  }

  // write key and iv to the file, and close it
  //
  // NB: we are ignoring errors here... good code wouldn't
  fwrite(key, sizeof(unsigned char), sizeof(key), file);
  fwrite(iv, sizeof(unsigned char), sizeof(iv), file);
  fclose(file);
}