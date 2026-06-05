#ifndef CONNECTION_H
#define CONNECTION_H

int conn_create(const char *port);
int conn_run(int server_fd);

#endif
