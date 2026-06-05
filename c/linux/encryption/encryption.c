#include "encryption.h"
#include <stdlib.h>
#include <string.h>

void user_init(User *u, const char *username, const char *password)
{
    strncpy(u->username, username, sizeof(u->username) - 1);
    hash_password((unsigned char*)password, strlen(password), u->password_hash);
    generate_key(u->key);
    u->token[0] = 0;
}

int user_check_password(User *u, const char *password)
{
    unsigned char hash[HASH_SIZE];
    hash_password((unsigned char*)password, strlen(password), hash);
    return memcmp(hash, u->password_hash, HASH_SIZE) == 0;
}

int msg_encrypt(const char *plaintext, User *recipient,
                unsigned char **out, int *out_len)
{
    int pt_len = strlen(plaintext);
    unsigned char *ciphertext = malloc(pt_len + 64);
    unsigned char iv[IV_SIZE], tag[TAG_SIZE];

    int ct_len = encrypt_msg((unsigned char*)plaintext, pt_len,
                              recipient->key, ciphertext, iv, tag);
    if (ct_len < 0) { free(ciphertext); return -1; }

    *out_len = IV_SIZE + TAG_SIZE + ct_len;
    *out = malloc(*out_len);
    memcpy(*out, iv, IV_SIZE);
    memcpy(*out + IV_SIZE, tag, TAG_SIZE);
    memcpy(*out + IV_SIZE + TAG_SIZE, ciphertext, ct_len);
    free(ciphertext);
    return 0;
}

int msg_decrypt(const unsigned char *data, int data_len, User *user,
                char **plaintext)
{
    if (data_len < IV_SIZE + TAG_SIZE) return -1;

    const unsigned char *iv = data;
    const unsigned char *tag = data + IV_SIZE;
    const unsigned char *ciphertext = data + IV_SIZE + TAG_SIZE;
    int ct_len = data_len - IV_SIZE - TAG_SIZE;

    unsigned char *pt_buf = malloc(ct_len + 16);
    int pt_len = decrypt_msg(ciphertext, ct_len, user->key, iv, tag, pt_buf);
    if (pt_len < 0) { free(pt_buf); return -1; }

    pt_buf[pt_len] = 0;
    *plaintext = (char*)pt_buf;
    return pt_len;
}
