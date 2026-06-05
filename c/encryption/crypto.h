#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>

#define KEY_SIZE 32
#define IV_SIZE 12
#define TAG_SIZE 16
#define HASH_SIZE 32

int  encrypt_msg(const unsigned char *plaintext, size_t pt_len,
                 const unsigned char *key,
                 unsigned char *ciphertext, unsigned char *iv, unsigned char *tag);

int  decrypt_msg(const unsigned char *ciphertext, size_t ct_len,
                 const unsigned char *key, const unsigned char *iv,
                 const unsigned char *tag, unsigned char *plaintext);

void generate_key(unsigned char *key);
void hash_password(const unsigned char *password, size_t len, unsigned char *hash);

#endif
