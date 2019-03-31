// [303] In assignment #1, you will need to complete aes_crypt_message().  The
//       reference solution requires under 50 lines of code.

#pragma once

#include <cstring>
#include <iostream>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <string>
#include <sys/stat.h>

#include "common_buf.h"

using namespace std;

/** size of RSA key */
const int RSA_KEYSIZE = 2048;

/** size of AES key */
const int AES_KEYSIZE = 32;

/** size of blocks that get encrypted... also size of IV */
const int AES_BLOCKSIZE = 16;

/** chunk size for reading/writing from files */
const int AES_BUFSIZE = 1024;

/**
 * Load an RSA public key from the given filename
 *
 * @param filename The name of the file that has the public key in it
 *
 * @returns An RSA context for encrypting with the provided public key, or
 *          nullptr on error
 */
RSA *load_pub(const char *filename) {
  FILE *pub = fopen(filename, "r");
  if (pub == nullptr) {
    std::cerr << "Error opening public key file\n";
    return nullptr;
  }
  RSA *rsa = PEM_read_RSAPublicKey(pub, nullptr, nullptr, nullptr);
  if (rsa == nullptr) {
    fclose(pub);
    std::cerr << "Error reading public key file\n";
    return nullptr;
  }
  return rsa;
}

/**
 * Load an RSA private key from the given filename
 *
 * @param filename The name of the file that has the private key in it
 *
 * @returns An RSA context for encrypting with the provided private key, or
 *          nullptr on error
 */
RSA *load_pri(const char *filename) {
  FILE *pri = fopen(filename, "r");
  if (pri == nullptr) {
    std::cerr << "Error opening private key file\n";
    return nullptr;
  }
  RSA *rsa = PEM_read_RSAPrivateKey(pri, nullptr, nullptr, nullptr);
  if (rsa == nullptr) {
    fclose(pri);
    std::cerr << "Error reading public key file\n";
    return nullptr;
  }
  return rsa;
}

/**
 * Produce an RSA key and save its public and private parts to files
 *
 * @param pub The name of the public key file to generate
 * @param pri The name of the private key file to generate
 *
 * @returns true on success, false on any error
 */
bool generate_rsa_key_files(std::string pub, std::string pri) {
  std::cout << "Generating RSA keys as (" << pub << ", " << pri << ")\n";
  // When we create a new RSA keypair, we need to know the #bits (see constant
  // above) and the desired exponent to use in the public key.  The exponent
  // needs to be a bignum.  We'll use the RSA_F4 default value:
  BIGNUM *bn = BN_new();
  if (bn == nullptr) {
    std::cerr << "Error in BN_new()\n";
    return false;
  }
  if (BN_set_word(bn, RSA_F4) != 1) {
    BN_free(bn);
    std::cerr << "Error in BN_set_word()\n";
    return false;
  }

  // Now we can create the key pair
  RSA *rsa = RSA_new();
  if (rsa == nullptr) {
    BN_free(bn);
    std::cerr << "Error in RSA_new()\n";
    return false;
  }
  if (RSA_generate_key_ex(rsa, RSA_KEYSIZE, bn, nullptr) != 1) {
    BN_free(bn);
    RSA_free(rsa);
    std::cerr << "Error in RSA_genreate_key_ex()\n";
    return false;
  }

  // Create/truncate the files
  FILE *pubfile = fopen(pub.c_str(), "w");
  if (pubfile == nullptr) {
    BN_free(bn);
    RSA_free(rsa);
    std::cerr << "Error opening public key file for output\n";
    return false;
  }
  FILE *prifile = fopen(pri.c_str(), "w");
  if (prifile == nullptr) {
    BN_free(bn);
    RSA_free(rsa);
    fclose(pubfile);
    std::cerr << "Error opening private key file for output\n";
    return false;
  }

  // Perform the writes.  Defer cleanup on error, because the cleanup is the
  // same
  bool res = true;
  if (PEM_write_RSAPublicKey(pubfile, rsa) != 1) {
    std::cerr << "Error writing public key\n";
    res = false;
  } else if (PEM_write_RSAPrivateKey(prifile, rsa, nullptr, nullptr, 0, nullptr,
                                     nullptr) != 1) {
    std::cerr << "Error writing private key\n";
    res = false;
  }

  // Cleanup regardless of whether the writes succeeded or failed
  fclose(pubfile);
  fclose(prifile);
  BN_free(bn);
  RSA_free(rsa);
  return res;
}

/**
 * Run the AES symmetric encryption/decryption algorithm on a buffer of bytes.
 * Note that this will do either encryption or decryption, depending on how the
 * provided CTX has been configured.  After calling, the CTX cannot be used
 * until it is reset.
 *
 * @param ctx The pre-configured AES context to use for this operatoin
 * @param msg A buffer of bytes to encrypt/decrypt
 *
 * @returns A buf_t with the encrypted or decrypted result, or -1 on error
 */
buf_t aes_crypt_msg(EVP_CIPHER_CTX *ctx, buf_t msg) {
  // NB: In C++, we can return a struct with this nice clean syntax.  This
  //     struct will have -1 as its 'num' field, which is a useful way to
  //     indicate an error.

    if(msg.num <= 0) return {-1, nullptr};

    int cipher_block_size = EVP_CIPHER_block_size(EVP_CIPHER_CTX_cipher(ctx));
    const int in_size = AES_BUFSIZE; 

    unsigned char out_buf[AES_BUFSIZE + cipher_block_size];
    memset(&out_buf, 0, AES_BUFSIZE + cipher_block_size+1);
    list<buf_t> sum;
    int done = 0;
    int remaining = msg.num;
    int out_len = 0;
    int num_to_read = 0;

    while(true){
        if(remaining < in_size){
            num_to_read = remaining;
        }
        else{
            num_to_read = in_size;
        }
        
        unsigned char *in_buf = (unsigned char *)malloc(num_to_read+1);
        memset(in_buf, 0, num_to_read+1);
        memcpy(in_buf, msg.bytes+done, num_to_read);
        done += num_to_read;
        remaining -= num_to_read;
        // crypt in_buf into out_buf
        if (!EVP_CipherUpdate(ctx, out_buf, &out_len, in_buf, num_to_read)) {
            std::cerr << "Error in EVP_CipherUpdate: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
            return {-1, nullptr};
        }
        // cout << "out_len " << out_len << endl;
        
        unsigned char *temp = (unsigned char *)malloc(out_len+1);
        memset(temp, 0, out_len+1);
        memcpy(temp, out_buf, out_len);
        buf_t temp2(out_len, temp);
        sum.push_back(temp2);
        
        if (num_to_read < in_size){
            break;
        }
    }

    // The final block needs special attention!
    if (!EVP_CipherFinal_ex(ctx, out_buf, &out_len)) {
        std::cerr << "Error in EVP_CipherFinal_ex: " << ERR_error_string(ERR_get_error(), nullptr);
        return {-1, nullptr};
    }

    // cout << "final out_len " << out_len << endl;
    unsigned char *temp = (unsigned char *)malloc(out_len+1);
    memset(temp, 0, out_len+1);
    memcpy(temp, out_buf, out_len);
    buf_t temp2(out_len, temp);
    sum.push_back(temp2);

    return buf_add_list(sum);
}

/**
 * Create an AES key.  A key is two parts, the key itself, and the
 * initialization vector.  Each is just random bits.  Our key will just be a
 * stream of random bits, long enough to be split into the actual key and the
 * iv.
 *
 * @returns a buf_t holding the key and iv bits
 */
buf_t create_aes_key() {
  unsigned char *key = (unsigned char *)malloc(AES_KEYSIZE + AES_BLOCKSIZE);
  if (!RAND_bytes(key, AES_KEYSIZE) ||
      !RAND_bytes(key + AES_KEYSIZE, AES_BLOCKSIZE)) {
    std::cerr << "Error in RAND_bytes()\n";
    free(key);
    return {-1, nullptr};
  }
  return {AES_KEYSIZE + AES_BLOCKSIZE, key};
}

/**
 * Create an aes context for doing a single encryption or decryption.  The
 * context must be reset after each full encrypt/decrypt.
 *
 * @param key     The bits of the key and iv, in a single array
 * @param encrypt True to encrypt, false to decrypt
 *
 * @returns An AES context for doing encryption.  Note that the context can be
 *          reset in order to re-use this object for another encryption.
 */
EVP_CIPHER_CTX *create_aes_context(unsigned char *key, bool encrypt) {
  if (!key) {
    std::cerr << "Incorrect key parameter to create_aes_context()\n";
    return nullptr;
  }

  // create and initialize a context for the AES operations we are going to do
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (ctx == nullptr) {
    std::cerr << "Error: OpenSSL couldn't create context: "
              << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    return nullptr;
  }

  // Make sure the key and iv lengths we have up above are valid
  if (!EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), nullptr, nullptr, nullptr,
                         encrypt)) {
    std::cerr << "Error: OpenSSL couldn't initialize context: "
              << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    EVP_CIPHER_CTX_cleanup(ctx);
    return nullptr;
  }
  if ((EVP_CIPHER_CTX_key_length(ctx) != AES_KEYSIZE) ||
      (EVP_CIPHER_CTX_iv_length(ctx) != AES_BLOCKSIZE)) {
    std::cerr << "Error: OpenSSL couldn't initialize context: "
              << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    EVP_CIPHER_CTX_cleanup(ctx);
    return nullptr;
  }

  // Set the key and iv on the AES context, and set the mode to encrypt or
  // decrypt
  if (!EVP_CipherInit_ex(ctx, nullptr, nullptr, key, key + AES_KEYSIZE,
                         encrypt)) {
    std::cerr << "Error: OpenSSL couldn't re-init context: "
              << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    EVP_CIPHER_CTX_cleanup(ctx);
    return nullptr;
  }
  return ctx;
}

/**
 * Reset an existing AES context, so that we can use it for another
 * encryption/decryption
 *
 * @param ctx     The AES context to reset
 * @param key     A stream of random bits to use as the key and iv.  Should be
 *                generated by create_aes_key().
 * @param encrypt True to create an encryption context, false to create a
 *                decryption context
 *
 * @returns false on error, true if the context is reset and ready to use again
 */
bool reset_aes_context(EVP_CIPHER_CTX *ctx, unsigned char *key, bool encrypt) {
  if (!EVP_CipherInit_ex(ctx, nullptr, nullptr, key, key + AES_KEYSIZE,
                         encrypt)) {
    std::cerr << "Error: OpenSSL couldn't re-init context: "
              << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    EVP_CIPHER_CTX_cleanup(ctx);
    return false;
  }
  return true;
}

/**
 * When an AES context is done being used, call this to reclaim its memory
 *
 * @param ctx The context to reclaim
 */
void reclaim_aes_context(EVP_CIPHER_CTX *ctx) { EVP_CIPHER_CTX_cleanup(ctx); }

/**
 * Use a server's public RSA key to encrypt the bits of the client's AES key
 *
 * @param pubkey The server's public key
 * @param key_iv The client's AES key and initialization vector, as a single
 *               byte stream
 *
 * @returns The encrypted AES key, or -1 on error
 */
buf_t make_encrypted_key(RSA *pubkey, unsigned char *key_iv) {
  buf_t res(RSA_size(pubkey));
  // Encrypt it into this buffer, with a size determined by the key
  memset(res.bytes, 0, RSA_size(pubkey));
  res.num = RSA_public_encrypt(AES_KEYSIZE + AES_BLOCKSIZE, key_iv, res.bytes,
                               pubkey, RSA_PKCS1_OAEP_PADDING);
  if (res.num == -1) {
    std::cerr << "Error producing encrypted key\n";
    free(res.bytes);
  }
  return res;
}

buf_t decrypted_rsakey(RSA *prikey, unsigned char *key_iv){
  buf_t res(RSA_size(prikey));
  // Decrypt it into this buffer, with a size determined by the key
  memset(res.bytes, 0, RSA_size(prikey));
  res.num = RSA_private_decrypt(256, key_iv, res.bytes, 
                                prikey, RSA_PKCS1_OAEP_PADDING);
  if (res.num == -1) {
    std::cerr << "Error producing decrypted key\n";
    free(res.bytes);
  }
  return res;
}

/**
 * If the given basename resolves to basename.pri and basename.pub, then load
 * basename.pri and return it.  If one or the other doesn't exist, then there's
 * an error.  If both don't exist, create them and then load basename.pri.
 *
 * @param basename The basename of the .pri and .pub files for RSA
 *
 * @returns The RSA context from loading the private file, or nullptr on error
 */
RSA *init_RSA(std::string basename) {
  std::string pubfile = basename + ".pub", prifile = basename + ".pri";

  struct stat stat_buf;
  bool pub_exists = (stat(pubfile.c_str(), &stat_buf) == 0);
  bool pri_exists = (stat(prifile.c_str(), &stat_buf) == 0);

  if (!pub_exists && !pri_exists) {
    generate_rsa_key_files(pubfile, prifile);
  } else if (pub_exists && !pri_exists) {
    std::cerr << "Error: cannot find " << basename << ".pri\n";
    return nullptr;
  } else if (!pub_exists && pri_exists) {
    std::cerr << "Error: cannot find " << basename << ".pub\n";
    return nullptr;
  }
  return load_pri(prifile.c_str());
}