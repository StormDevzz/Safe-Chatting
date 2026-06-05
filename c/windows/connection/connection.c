#include "connection.h"
#include "../common.h"
#include "../chatting/chatting.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

static void set_nonblock(SOCKET fd)
{
    u_long mode = 1;
    ioctlsocket(fd, FIONBIO, &mode);
}

int conn_create(const char *port)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        { fprintf(stderr, "WSAStartup failed\n"); return -1; }

    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
        { fprintf(stderr, "socket failed\n"); WSACleanup(); return -1; }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(port));

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        { fprintf(stderr, "bind failed\n"); closesocket(fd); WSACleanup(); return -1; }
    if (listen(fd, 10) == SOCKET_ERROR)
        { fprintf(stderr, "listen failed\n"); closesocket(fd); WSACleanup(); return -1; }

    set_nonblock(fd);
    return (int)fd;
}

int conn_run(int server_fd)
{
    printf("SafeChatting on port %s\n", PORT);

    while (1) {
        WSAPOLLFD fds[128];
        int nfds = 1;
        fds[0].fd = server_fd;
        fds[0].events = POLLIN;

        for (int i = 0; i < 128; i++) {
            Client *c = chat_get_client(i);
            if (!c) break;
            if (c->fd >= 0) {
                fds[nfds].fd = c->fd;
                fds[nfds].events = POLLIN;
                nfds++;
            }
        }

        int ret = WSAPoll(fds, nfds, -1);
        if (ret == SOCKET_ERROR) break;

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in ca;
            int al = sizeof(ca);
            SOCKET new_fd = accept(server_fd, (struct sockaddr*)&ca, &al);
            if (new_fd != INVALID_SOCKET) {
                set_nonblock(new_fd);
                printf("Connected\n");
                if (chat_add_client((int)new_fd) >= 0)
                    chat_send((int)new_fd, RSP_OK, "Welcome! Use LOGIN|user|pass or REGISTER|user|pass");
                else
                    closesocket(new_fd);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                char buf[4096 * 4];
                int n = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);
                if (n <= 0) {
                    closesocket(fds[i].fd);
                    chat_remove_client(i - 1);
                    continue;
                }
                buf[n] = 0;

                char *line = buf;
                while (line && *line) {
                    char *nl = strchr(line, '\n');
                    if (nl) *nl = 0;
                    if (*line) chat_handle(i - 1, line);
                    if (nl) line = nl + 1; else break;
                }
            }
        }
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
