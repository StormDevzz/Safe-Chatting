#ifndef CHATTING_H
#define CHATTING_H

#include "../encryption/encryption.h"

typedef struct {
    int  fd;
    int  authenticated;
    char username[64];
    char token[64];
} Client;

int  chat_init(void);
int  chat_register(const char *user, const char *pass);
int  chat_login(const char *user, const char *pass, char *token, int *user_idx);
void chat_handle(int client_idx, const char *line);
void chat_deliver_offline(int client_idx);
int  chat_find_user(const char *username);
User *chat_get_user(int idx);
Client *chat_get_client(int idx);
void chat_send(int fd, const char *type, const char *data);
int  chat_add_client(int fd);
void chat_remove_client(int idx);

#endif
