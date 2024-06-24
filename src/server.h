#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include "hashtable.h"

#define MAX_CLIENTS 2

typedef struct {
    struct sockaddr_in addr;
    int stop;
    int master_socket;
    int client_sockets[MAX_CLIENTS];
    hash_table *table;
} server_t;

server_t *server_new(short port, int backlog);
void server_run(server_t *server);
void server_stop(server_t *server);

#endif // SERVER_H
