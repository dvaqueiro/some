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

server_t *server_init(short port, int backlog);
void server_free(server_t *server);
void server_main_loop(server_t *server);
void server_accept_new_connection(server_t *server, char *welcome);
void server_print_sockets(server_t *server);

#endif // SERVER_H
