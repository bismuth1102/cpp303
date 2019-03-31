#include <iostream>

#include "common_crypto.h"

using namespace std;

buf_t encrypt_length(buf_t aes_key, buf_t encrypted_msg, EVP_CIPHER_CTX * ctx){
	unsigned char length = encrypted_msg.num;
	buf_t buf_length(4, &length);
	buf_t encrypted_length = aes_crypt_msg(ctx, buf_length);
	cout << "encrypted_length.num: " << encrypted_length.num << endl;

	reset_aes_context(ctx, aes_key.bytes, false);
	buf_t msg = aes_crypt_msg(ctx, encrypted_length);
	cout << (int)*msg.bytes << endl;


	return encrypted_length;
}

int main(){
	buf_t test = buf_t("12345678901234567890123456789012");

	buf_t aes_key = create_aes_key();
	EVP_CIPHER_CTX *ctx = create_aes_context(aes_key.bytes, true);
	// buf_t encrypted_msg = aes_crypt_msg(ctx, test);
	// reset_aes_context(ctx, aes_key.bytes, false);

	// buf_t msg = aes_crypt_msg(ctx, encrypted_msg);
	// reclaim_aes_context(ctx);

	// cout << string((char *)msg.bytes, msg.num) << endl;

	encrypt_length(aes_key, test, ctx);

}

