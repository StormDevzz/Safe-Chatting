#include "../common.h"
#include "../connection/connection.h"
#include "../chatting/chatting.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char **argv)
{
    srand(time(NULL));
    chat_init();

    const char *port = PORT;
    if (argc > 1) port = argv[1];

    int fd = conn_create(port);
    if (fd < 0) return 1;

    printf("\033[1;35mSafeChatting\033[0m на порту \033[36m%s\033[0m\n", port);
    return conn_run(fd);
}
