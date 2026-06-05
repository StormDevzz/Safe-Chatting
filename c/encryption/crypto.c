#include "crypto.h"
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

int encrypt_msg(const unsigned char *plaintext, size_t pt_len,
                const unsigned char *key,
                unsigned char *ciphertext, unsigned char *iv, unsigned char *tag)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (RAND_bytes(iv, IV_SIZE) != 1) { EVP_CIPHER_CTX_free(ctx); return -1; }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1)
        { EVP_CIPHER_CTX_free(ctx); return -1; }

    int len = 0, ct_len = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, (int)pt_len) != 1)
        { EVP_CIPHER_CTX_free(ctx); return -1; }
    ct_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + ct_len, &len) != 1)
        { EVP_CIPHER_CTX_free(ctx); return -1; }
    ct_len += len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1)
        { EVP_CIPHER_CTX_free(ctx); return -1; }

    EVP_CIPHER_CTX_free(ctx);
    return ct_len;
}

int decrypt_msg(const unsigned char *ciphertext, size_t ct_len,
                const unsigned char *key, const unsigned char *iv,
                const unsigned char *tag, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1)
        { EVP_CIPHER_CTX_free(ctx); return -1; }

    int len = 0, pt_len = 0;
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, (int)ct_len) != 1)
        { EVP_CIPHER_CTX_free(ctx); return -1; }
    pt_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, (void*)tag) != 1)
        { EVP_CIPHER_CTX_free(ctx); return -1; }

    int ret = EVP_DecryptFinal_ex(ctx, plaintext + pt_len, &len);
    EVP_CIPHER_CTX_free(ctx);
    if (ret != 1) return -1;
    return pt_len + len;
}

void generate_key(unsigned char *key) { RAND_bytes(key, KEY_SIZE); }

void hash_password(const unsigned char *password, size_t len, unsigned char *hash)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, password, len);
    EVP_DigestFinal_ex(ctx, hash, NULL);
    EVP_MD_CTX_free(ctx);
}
