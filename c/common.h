#ifndef COMMON_H
#define COMMON_H

#define PORT "8080"
#define MAX_LINE 4096
#define MAX_USER 64
#define MAX_MSG 2048
#define TOKEN_LEN 64
#define MAX_CLIENTS 128
#define STORAGE_DIR "./storage"

#define CMD_REGISTER "REGISTER"
#define CMD_LOGIN    "LOGIN"
#define CMD_SEND     "SEND"
#define CMD_LIST     "LIST"

#define RSP_OK       "OK"
#define RSP_MSG      "MSG"
#define RSP_ERR      "ERR"
#define RSP_USERS    "USERS"

#endif
