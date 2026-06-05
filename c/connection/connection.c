#include "connection.h"
#include "../common.h"
#include "../chatting/chatting.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

static void set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int conn_create(const char *port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(port));

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        { perror("bind"); close(fd); return -1; }
    if (listen(fd, 10) < 0)
        { perror("listen"); close(fd); return -1; }

    set_nonblock(fd);
    return fd;
}

int conn_run(int server_fd)
{
    struct pollfd fds[MAX_CLIENTS + 1];

    printf("\033[1;35m"
" SSSSSSSSSSSSSSS                      ffffffffffffffff                               CCCCCCCCCCCCChhhhhhh                                       tttt               tttt            iiii                                       \n"
" SS:::::::::::::::S                    f::::::::::::::::f                           CCC::::::::::::Ch:::::h                                    ttt:::t            ttt:::t           i::::i                                      \n"
"S:::::SSSSSS::::::S                   f::::::::::::::::::f                        CC:::::::::::::::Ch:::::h                                    t:::::t            t:::::t            iiii                                       \n"
"S:::::S     SSSSSSS                   f::::::fffffff:::::f                       C:::::CCCCCCCC::::Ch:::::h                                    t:::::t            t:::::t                                                       \n"
"S:::::S              aaaaaaaaaaaaa    f:::::f       ffffffeeeeeeeeeeee          C:::::C       CCCCCC h::::h hhhhh         aaaaaaaaaaaaa  ttttttt:::::tttttttttttttt:::::ttttttt    iiiiiiinnnn  nnnnnnnn       ggggggggg   ggggg\n"
"S:::::S              a::::::::::::a   f:::::f           ee::::::::::::ee       C:::::C               h::::hh:::::hhh      a::::::::::::a t:::::::::::::::::tt:::::::::::::::::t    i:::::in:::nn::::::::nn    g:::::::::ggg::::g\n"
" S::::SSSS           aaaaaaaaa:::::a f:::::::ffffff    e::::::eeeee:::::ee     C:::::C               h::::::::::::::hh    aaaaaaaaa:::::at:::::::::::::::::tt:::::::::::::::::t     i::::in::::::::::::::nn  g:::::::::::::::::g\n"
"  SS::::::SSSSS               a::::a f::::::::::::f   e::::::e     e:::::e     C:::::C               h:::::::hhh::::::h            a::::atttttt:::::::tttttttttttt:::::::tttttt     i::::inn:::::::::::::::ng::::::ggggg::::::gg\n"
"    SSS::::::::SS      aaaaaaa:::::a f::::::::::::f   e:::::::eeeee::::::e     C:::::C               h::::::h   h::::::h    aaaaaaa:::::a      t:::::t            t:::::t           i::::i  n:::::nnnn:::::ng:::::g     g:::::g \n"
"       SSSSSS::::S   aa::::::::::::a f:::::::ffffff   e:::::::::::::::::e      C:::::C               h:::::h     h:::::h  aa::::::::::::a      t:::::t            t:::::t           i::::i  n::::n    n::::ng:::::g     g:::::g \n"
"            S:::::S a::::aaaa::::::a  f:::::f         e::::::eeeeeeeeeee       C:::::C               h:::::h     h:::::h a::::aaaa::::::a      t:::::t            t:::::t           i::::i  n::::n    n::::ng:::::g     g:::::g \n"
"            S:::::Sa::::a    a:::::a  f:::::f         e:::::::e                 C:::::C       CCCCCC h:::::h     h:::::ha::::a    a:::::a      t:::::t    tttttt  t:::::t    tttttt i::::i  n::::n    n::::ng::::::g    g:::::g \n"
"SSSSSSS     S:::::Sa::::a    a:::::a f:::::::f        e::::::::e                 C:::::CCCCCCCC::::C h:::::h     h:::::ha::::a    a:::::a      t::::::tttt:::::t  t::::::tttt:::::ti::::::i n::::n    n::::ng:::::::ggggg:::::g \n"
"S::::::SSSSSS:::::Sa:::::aaaa::::::a f:::::::f         e::::::::eeeeeeee          CC:::::::::::::::C h:::::h     h:::::ha:::::aaaa::::::a      tt::::::::::::::t  tt::::::::::::::ti::::::i n::::n    n::::n g::::::::::::::::g \n"
"S:::::::::::::::SS  a::::::::::aa:::af:::::::f          ee:::::::::::::e            CCC::::::::::::C h:::::h     h:::::h a::::::::::aa:::a       tt:::::::::::tt    tt:::::::::::tti::::::i n::::n    n::::n  gg::::::::::::::g \n"
" SSSSSSSSSSSSSSS     aaaaaaaaaa  aaaafffffffff            eeeeeeeeeeeeee               CCCCCCCCCCCCC hhhhhhh     hhhhhhh  aaaaaaaaaa  aaaa         ttttttttttt        ttttttttttt  iiiiiiii nnnnnn    nnnnnn    gggggggg::::::g \n"
"                                                                                                                                                                                                                        g:::::g \n"
"                                                                                                                                                                                                            gggggg      g:::::g \n"
"                                                                                                                                                                                                            g:::::gg   gg:::::g \n"
"                                                                                                                                                                                                             g::::::ggg:::::::g \n"
"                                                                                                                                                                                                              gg:::::::::::::g  \n"
"                                                                                                                                                                                                                ggg::::::ggg    \n"
"                                                                                                                                                                                                                   gggggg      \n"
"\033[0m"
           "\033[36m  ./chat — подключиться | @user текст — написать\033[0m\n");

    while (1) {
        int nfds = 1;
        fds[0].fd = server_fd;
        fds[0].events = POLLIN;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            Client *c = chat_get_client(i);
            if (!c) break;
            if (c->fd >= 0) {
                fds[nfds].fd = c->fd;
                fds[nfds].events = POLLIN;
                nfds++;
            }
        }

        int ret = poll(fds, nfds, -1);
        if (ret < 0) { perror("poll"); break; }

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in ca;
            socklen_t al = sizeof(ca);
            int new_fd = accept(server_fd, (struct sockaddr*)&ca, &al);
            if (new_fd >= 0) {
                set_nonblock(new_fd);
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ca.sin_addr, ip, sizeof(ip));
                printf("✦ Подключился: %s\n", ip);

                if (chat_add_client(new_fd) >= 0)
                    chat_send(new_fd, RSP_OK,
                              "Welcome! Use LOGIN|user|pass or REGISTER|user|pass");
                else
                    close(new_fd);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                char buf[MAX_LINE * 4];
                int n = read(fds[i].fd, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    close(fds[i].fd);
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

    close(server_fd);
    return 0;
}
