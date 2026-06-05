#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include "crypto.h"

typedef struct {
    char username[64];
    unsigned char password_hash[HASH_SIZE];
    unsigned char key[KEY_SIZE];
    char token[64];
} User;

void  user_init(User *u, const char *username, const char *password);
int   user_check_password(User *u, const char *password);
int   msg_encrypt(const char *plaintext, User *recipient,
                  unsigned char **out, int *out_len);
int   msg_decrypt(const unsigned char *data, int data_len, User *user,
                  char **plaintext);

#endif
