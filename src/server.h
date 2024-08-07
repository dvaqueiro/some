#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include "database.h"

typedef struct {
    struct sockaddr_in addr;
    int max_clients;
    int current_clients;
    int stop;
    int master_socket;
    database *db;
    int client_sockets[];
} server_t;

server_t *server_new(short port, int backlog, int max_clients);
void server_run(server_t *server);
void server_stop(server_t *server);
int server_status(server_t *server, char *buff, size_t buff_size);

#endif // SERVER_H
