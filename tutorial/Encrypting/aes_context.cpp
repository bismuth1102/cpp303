/**
 * Produce an AES context that can be used for encrypting or decrypting.  Set
 * the AES key from the provided file.
 *
 * @param keyfile The name of the file holding the AES key
 * @param encrypt true if the context will be used to encrypt, false otherwise
 *
 * @return A properly configured AES context
 */
EVP_CIPHER_CTX *get_aes_context(std::string keyfile, bool encrypt) {
  // Open the key file and read the key and iv
  FILE *file = fopen(keyfile.c_str(), "rb");
  if (!file) {
    perror("Error opening keyfile");
    exit(0);
  }
  unsigned char key[AES_256_KEY_SIZE], iv[BLOCK_SIZE];
  int num_read = fread(key, sizeof(unsigned char), sizeof(key), file);
  num_read += fread(iv, sizeof(unsigned char), sizeof(iv), file);
  fclose(file);
  if (num_read != AES_256_KEY_SIZE + BLOCK_SIZE) {
    fprintf(stderr, "Error reading keyfile");
    exit(0);
  }

  // create and initialize a context for the AES operations we are going to do
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (ctx == nullptr) {
    fprintf(stderr, "Error: OpenSSL couldn't create context: %s\n",
            ERR_error_string(ERR_get_error(), nullptr));
    exit(0);
  }

  // Make sure the key and iv lengths we have up above are valid
  if (!EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), nullptr, nullptr, nullptr,
                         1)) {
    fprintf(stderr, "Error: OpenSSL couldn't initialize context: %s\n",
            ERR_error_string(ERR_get_error(), nullptr));
    exit(0);
  }
  OPENSSL_assert(EVP_CIPHER_CTX_key_length(ctx) == AES_256_KEY_SIZE);
  OPENSSL_assert(EVP_CIPHER_CTX_iv_length(ctx) == BLOCK_SIZE);

  // Set the key and iv on the AES context, and set the mode to encrypt or
  // decrypt
  if (!EVP_CipherInit_ex(ctx, nullptr, nullptr, key, iv, encrypt ? 1 : 0)) {
    fprintf(stderr, "Error: OpenSSL couldn't re-init context: %s\n",
            ERR_error_string(ERR_get_error(), nullptr));
    EVP_CIPHER_CTX_cleanup(ctx);
    exit(0);
  }

  return ctx;
}