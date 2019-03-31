#include <cstring>
#include <iostream>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <string>
#include <sys/stat.h>

using namespace std;

/** size of AES key */
const int AES_256_KEY_SIZE = 32;

/** size of blocks that get encrypted... also size of IV */
const int BLOCK_SIZE = 16;

/** chunk size for reading/writing from files */
const int BUFSIZE = 1024;

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
    cout << "num_bytes_read " << num_bytes_read << endl;
    cout << "in_buf " << strlen((const char*)in_buf) << endl;
    // crypt in_buf into out_buf
    if (!EVP_CipherUpdate(ctx, out_buf, &out_len, in_buf, num_bytes_read)) {
      fprintf(stderr, "Error in EVP_CipherUpdate: %s\n",
              ERR_error_string(ERR_get_error(), nullptr));
      return false;
    }
    cout << "out_len " << out_len << endl;
    cout << "out_buf " << strlen((const char*)out_buf) << endl;
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
  cout << "final out_len " << out_len << endl;
  cout << "final out_buf " << strlen((const char*)out_buf) << endl;
  fwrite(out_buf, sizeof(unsigned char), out_len, out);
  if (ferror(out)) {
    perror("Error in fwrite");
    return false;
  }
  return true;
}