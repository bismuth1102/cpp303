/**
 * Take an input file and run the AES algorithm on it to produce an output file.
 * This can be used for either encryption or decryption, depending on how ctx is
 * configured.
 *
 * @param ctx A fully-configured symmetric cipher object for managing the
 *            encryption or decryption
 * @param in  the file to read
 * @param out the file to populate with the result of the AES algorithm
 */
bool aes_crypt(EVP_CIPHER_CTX *ctx, FILE *in, FILE *out) {
  // figure out the block size that AES is going to use
  int cipher_block_size = EVP_CIPHER_block_size(EVP_CIPHER_CTX_cipher(ctx));

  // Set up a buffer where AES puts crypted bits.  Since the last block is
  // special, we need this outside the loop.
  unsigned char out_buf[BUFSIZE + cipher_block_size];
  int out_len;

  // Read blocks from the file and crypt them:
  while (true) {
    // read from file
    unsigned char in_buf[BUFSIZE];
    int num_bytes_read = fread(in_buf, sizeof(unsigned char), BUFSIZE, in);
    if (ferror(in)) {
      perror("Error in fread()");
      return false;
    }
    // crypt in_buf into out_buf
    if (!EVP_CipherUpdate(ctx, out_buf, &out_len, in_buf, num_bytes_read)) {
      fprintf(stderr, "Error in EVP_CipherUpdate: %s\n",
              ERR_error_string(ERR_get_error(), nullptr));
      return false;
    }
    // write crypted bytes to file
    fwrite(out_buf, sizeof(unsigned char), out_len, out);
    if (ferror(out)) {
      perror("Error in fwrite()");
      return false;
    }
    // stop on EOF
    if (num_bytes_read < BUFSIZE) {
      break;
    }
  }

  // The final block needs special attention!
  if (!EVP_CipherFinal_ex(ctx, out_buf, &out_len)) {
    fprintf(stderr, "Error in EVP_CipherFinal_ex: %s\n",
            ERR_error_string(ERR_get_error(), nullptr));
    return false;
  }
  fwrite(out_buf, sizeof(unsigned char), out_len, out);
  if (ferror(out)) {
    perror("Error in fwrite");
    return false;
  }
  return true;
}