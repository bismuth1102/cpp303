/**
 * Encrypt a file's contents and write the result to another file
 *
 * @param pub The public key
 * @param in  The file to read
 * @param out The file to populate with the result of the encryption
 */
bool rsa_encrypt(RSA *pub, FILE *in, FILE *out) {
  // Note that RSA does not work on big files, only small stuff.  We're going to
  // assume that the file is small, and read it straight into this buffer:
  unsigned char msg[RSA_KEY_SIZE / 8] = {0};
  int bytes = fread(msg, 1, sizeof(msg), in);
  if (ferror(in)) {
    perror("Error in fread()");
    return false;
  }

  // Encrypt it into this buffer, with a size determined by the key
  unsigned char enc[RSA_size(pub)] = {0};
  int len = RSA_public_encrypt(bytes, msg, enc, pub, RSA_PKCS1_OAEP_PADDING);
  if (len == -1) {
    fprintf(stderr, "Error encrypting\n");
    return false;
  }

  // Write the result to the output file
  fwrite(enc, 1, len, out);
  if (ferror(out)) {
    perror("Error in fwrite()");
    return false;
  }
  return true;
}