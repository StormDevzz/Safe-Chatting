#include "chatting.h"
#include "../common.h"
#include "../files/files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

static Client clients[MAX_CLIENTS];
static User   users[MAX_CLIENTS];
static int    user_count = 0;
static int    client_count = 0;

int chat_init(void)
{
    user_count = 0;
    client_count = 0;
    mkdir(STORAGE_DIR, 0700);
    char dir[MAX_LINE];
    snprintf(dir, sizeof(dir), "%s/messages", STORAGE_DIR);
    mkdir(dir, 0700);
    return 0;
}

Client *chat_get_client(int idx)
{
    if (idx < 0 || idx >= client_count) return NULL;
    return &clients[idx];
}

User *chat_get_user(int idx)
{
    if (idx < 0 || idx >= user_count) return NULL;
    return &users[idx];
}

int chat_find_user(const char *username)
{
    for (int i = 0; i < user_count; i++)
        if (strcmp(users[i].username, username) == 0) return i;
    return -1;
}

int chat_register(const char *user, const char *pass)
{
    if (chat_find_user(user) >= 0 || user_count >= MAX_CLIENTS) return -1;
    user_init(&users[user_count++], user, pass);
    return 0;
}

int chat_login(const char *user, const char *pass, char *token, int *user_idx)
{
    int idx = chat_find_user(user);
    if (idx < 0) return -1;
    if (!user_check_password(&users[idx], pass)) return -1;

    snprintf(users[idx].token, TOKEN_LEN, "tok_%s_%d", user, rand());
    if (token) strncpy(token, users[idx].token, TOKEN_LEN - 1);
    if (user_idx) *user_idx = idx;
    return 0;
}

void chat_send(int fd, const char *type, const char *data)
{
    char buf[MAX_LINE];
    int n = snprintf(buf, sizeof(buf), "%s|%s\n", type, data);
    write(fd, buf, n);
}

void chat_handle(int ci, const char *line)
{
    Client *c = &clients[ci];
    char cmd[MAX_LINE], arg1[MAX_LINE], arg2[MAX_LINE];
    int parsed = sscanf(line, "%[^|]|%[^|]|%[^\n]", cmd, arg1, arg2);
    if (parsed < 1) return;

    if (strcmp(cmd, CMD_REGISTER) == 0 && parsed == 3) {
        if (chat_register(arg1, arg2) == 0)
            chat_send(c->fd, RSP_OK, "registered");
        else
            chat_send(c->fd, RSP_ERR, "user exists or server full");
    }
    else if (strcmp(cmd, CMD_LOGIN) == 0 && parsed >= 2) {
        char token[TOKEN_LEN] = {0};
        int ui = -1;
        if (chat_login(arg1, parsed >= 3 ? arg2 : "", token, &ui) == 0) {
            c->authenticated = 1;
            strncpy(c->username, arg1, sizeof(c->username) - 1);
            strncpy(c->token, token, sizeof(c->token) - 1);
            char resp[MAX_LINE];
            snprintf(resp, sizeof(resp), "logged in as %s", arg1);
            chat_send(c->fd, RSP_OK, resp);
            chat_deliver_offline(ci);
        } else {
            chat_send(c->fd, RSP_ERR, "invalid credentials");
        }
    }
    else if (strcmp(cmd, CMD_SEND) == 0 && parsed == 3) {
        if (!c->authenticated) {
            chat_send(c->fd, RSP_ERR, "not logged in");
            return;
        }
        int ri = chat_find_user(arg1);
        if (ri < 0) {
            chat_send(c->fd, RSP_ERR, "user not found");
            return;
        }

        // Deliver online
        int delivered = 0;
        for (int i = 0; i < client_count; i++) {
            if (clients[i].authenticated && strcmp(clients[i].username, arg1) == 0) {
                char msg[MAX_LINE];
                snprintf(msg, sizeof(msg), "%s|%s", c->username, arg2);
                chat_send(clients[i].fd, RSP_MSG, msg);
                delivered = 1;
                break;
            }
        }

        // Encrypt and store for offline
        unsigned char *enc = NULL;
        int elen = 0;
        msg_encrypt(arg2, &users[ri], &enc, &elen);
        if (enc) {
            time_t t = time(NULL);
            char fname[MAX_LINE];
            snprintf(fname, sizeof(fname), "%s/messages/%s/%s_%ld.msg",
                     STORAGE_DIR, arg1, c->username, t);
            char dir[MAX_LINE];
            snprintf(dir, sizeof(dir), "%s/messages/%s", STORAGE_DIR, arg1);
            mkdir(dir, 0700);

            FILE *fp = fopen(fname, "wb");
            if (fp) { fwrite(enc, 1, elen, fp); fclose(fp); }
            free(enc);
        }

        chat_send(c->fd, RSP_OK, delivered ? "delivered" : "stored");
    }
    else if (strcmp(cmd, CMD_LIST) == 0) {
        if (!c->authenticated) {
            chat_send(c->fd, RSP_ERR, "not logged in");
            return;
        }
        char list[MAX_LINE * 2] = {0};
        int first = 1;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, c->username) == 0) continue;
            int online = 0;
            for (int j = 0; j < client_count; j++)
                if (clients[j].authenticated && strcmp(clients[j].username, users[i].username) == 0)
                    { online = 1; break; }
            char entry[MAX_LINE];
            snprintf(entry, sizeof(entry), "%s%s:%s", first ? "" : ",", users[i].username, online ? "online" : "offline");
            strncat(list, entry, sizeof(list) - strlen(list) - 1);
            first = 0;
        }
        chat_send(c->fd, RSP_USERS, list);
    }
}

void chat_deliver_offline(int ci)
{
    Client *c = &clients[ci];
    if (!c->authenticated) return;

    char dir[MAX_LINE];
    snprintf(dir, sizeof(dir), "%s/messages/%s", STORAGE_DIR, c->username);

    DIR *dp = opendir(dir);
    if (!dp) return;

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_type != DT_REG) continue;

        char fname[MAX_LINE];
        snprintf(fname, sizeof(fname), "%s/%s", dir, entry->d_name);

        FILE *fp = fopen(fname, "rb");
        if (!fp) continue;
        fseek(fp, 0, SEEK_END);
        int len = ftell(fp);
        rewind(fp);
        unsigned char *data = malloc(len);
        fread(data, 1, len, fp);
        fclose(fp);

        int ui = chat_find_user(c->username);
        if (ui >= 0) {
            char *text = NULL;
            if (msg_decrypt(data, len, &users[ui], &text) > 0) {
                char sender[64] = {0};
                const char *us = strchr(entry->d_name, '_');
                if (us) {
                    int slen = us - entry->d_name;
                    if (slen > 0 && slen < 64) {
                        strncpy(sender, entry->d_name, slen);
                        sender[slen] = 0;
                    }
                }
                char msg[MAX_LINE];
                snprintf(msg, sizeof(msg), "%s|%s", sender, text);
                chat_send(c->fd, RSP_MSG, msg);
                free(text);
            }
        }
        free(data);
        remove(fname);
    }
    closedir(dp);
}

int chat_add_client(int fd)
{
    if (client_count >= MAX_CLIENTS) return -1;
    clients[client_count].fd = fd;
    clients[client_count].authenticated = 0;
    clients[client_count].username[0] = 0;
    clients[client_count].token[0] = 0;
    return client_count++;
}

void chat_remove_client(int idx)
{
    if (idx < 0 || idx >= client_count) return;
    clients[idx] = clients[--client_count];
}
