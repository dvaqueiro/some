#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include "server.h"
#include "command.h"
#include "hashtable.h"

#define SOCKET_ERROR -1
#define MAX_READ_BUFFER_SIZE 1024
#define MAX_WRITE_BUFFER_SIZE 1024

#define FNV_PRIME 0x10000001b3
#define FNV_OFFSET 0xcbf29ce48422325UL

typedef struct Request {
    int socket_idx;
    char *read_buffer;
    int read_buffer_size;
} Request;

int check_or_exit(int err, const char *msg) {
    if (err == SOCKET_ERROR) {
        perror(msg);
        exit(1);
    }
    return err;
}

uint64_t hash_fnv1(const char *key, size_t length) {
    uint64_t hash_value = FNV_OFFSET;
    for (int i = 0; i < length; i++) {
        hash_value ^= key[i];
        hash_value *= FNV_PRIME;
    }
    return hash_value;
}

void server_free(server_t *server) {
    close(server->master_socket);
    hash_table_destroy(server->table);
    free(server);
}

void server_hash_table_create(server_t *server) {
    server->table = hash_table_create(5, hash_fnv1, NULL);
}

/* The `socket(2)` syscall creates an endpoint for communication
 * and returns a file descriptor that refers to that endpoint.
 *
 * It takes three arguments (the last being just to provide greater
 * specificity):
 * -    domain (communication domain)
 *      AF_INET              IPv4 Internet protocols
 *
 * -    type (communication semantics)
 *      SOCK_STREAM          Provides sequenced, reliable,
 *                           two-way, connection-based byte
 *                           streams.
 */
server_t *server_new(short port, int backlog, int max_clients) {
    server_t *server = malloc(sizeof(server_t) + max_clients * sizeof(int));

    if (server == NULL) {
        perror("Failed to allocate memory for server");
        exit(1);
    }

    server_hash_table_create(server);
    server->max_clients = max_clients;
    server->current_clients = 0;
    server->stop = 0;

    server->master_socket = 0;
    for (int i = 0; i < max_clients; i++) {
        server->client_sockets[i] = 0;
    }
    bzero(&server->addr, sizeof(server->addr));

    check_or_exit(server->master_socket = socket(AF_INET, SOCK_STREAM, 0), "Failed to create master socket");
    check_or_exit(setsockopt(server->master_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)), "Failed to set master socket options");

    server->addr.sin_family = AF_INET;
    server->addr.sin_addr.s_addr = INADDR_ANY;
    server->addr.sin_port = htons(port);

    check_or_exit(bind(server->master_socket, (struct sockaddr*)&server->addr, sizeof(server->addr)), "Failed to bind master socket to address");

    check_or_exit(listen(server->master_socket, backlog), "Failed to put master socket on passive mode");
    printf("Waiting for connections on 0.0.0.0:%d\n", port);

    return server;
}

int server_status(server_t *server, char *buff, size_t buff_size) {
    int res_len = 0;

    res_len += snprintf(buff + res_len, buff_size - res_len, "# Server\n");
    res_len += snprintf(buff + res_len, buff_size - res_len, "Max clients: %i\n", server->max_clients);
    res_len += snprintf(buff + res_len, buff_size - res_len, "Current clients: %i\n", server->current_clients);
    res_len += snprintf(buff + res_len, buff_size - res_len, "# Sockets\n");
    res_len += snprintf(buff + res_len, buff_size - res_len, "Master socket: %d\n", server->master_socket);
    for (int i = 0; i < server->max_clients; i++) {
        res_len += snprintf(buff + res_len, buff_size - res_len, "Client socket [%d]: %d\n", i, server->client_sockets[i]);
    }
    res_len += snprintf(buff + res_len, buff_size - res_len, "# Hast Talbe\n");
    res_len += hash_table_status(server->table, buff + res_len, buff_size - res_len);

    return res_len;
}

void server_accept_new_connection(server_t *server) {
    int new_socket;
    int server_last_clients = server->current_clients;

    socklen_t client_len = sizeof(server->addr);
    if ((new_socket = accept(server->master_socket, (struct sockaddr *)&server->addr, &client_len)) == SOCKET_ERROR) {
        perror("accept");
        printf("Failed accepting connection\n");
    }
    //add new socket to array of sockets
    for (int i = 0; i < server->max_clients; i++) {
        if (server->client_sockets[i] == 0) {
            server->client_sockets[i] = new_socket;
            server->current_clients++;
            printf(
                    "Connection accepted (socket: %d) from ip:%s port:%d\n",
                    new_socket,
                    inet_ntoa(server->addr.sin_addr),
                    ntohs(server->addr.sin_port)
                  );
            break;
        }
    }

    if (server->current_clients == server_last_clients) {
        printf("No more connections allowed\n");
        close(new_socket);
    }
}

void server_handle_client_close(server_t *server, int client_socket_idx) {
    socklen_t addr_len = sizeof(server->addr);
    getpeername(server->client_sockets[client_socket_idx], (struct sockaddr*)&server->addr, &addr_len);
    printf(
            "Host disconnected (socked: %d), ip %s, port %d\n",
            server->client_sockets[client_socket_idx],
            inet_ntoa(server->addr.sin_addr),
            ntohs(server->addr.sin_port)
          );
    close(server->client_sockets[client_socket_idx]);
    server->client_sockets[client_socket_idx] = 0;
    server->current_clients--;
}

void server_handle_client_req(server_t *server, Request *req) {
    int res_len = 0;
    char response[MAX_WRITE_BUFFER_SIZE];
    bzero(response, MAX_WRITE_BUFFER_SIZE);
    req->read_buffer[strcspn(req->read_buffer, "\r\n")] = '\0';
    res_len = process_command(server, req->read_buffer, response, MAX_WRITE_BUFFER_SIZE);
    send(server->client_sockets[req->socket_idx], response, res_len, 0);
}

void server_run(server_t *server) {
    int max_sd, activity, valread;
    fd_set readfds;
    char buffer[MAX_READ_BUFFER_SIZE + 1];

    while (server->stop != 1) {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(server->master_socket, &readfds);

        //add child sockets to set
        max_sd = server->master_socket;
        for (int i = 0; i < server->max_clients; i++) {
            if (server->client_sockets[i] > 0) {
                FD_SET(server->client_sockets[i], &readfds);
            }
            if (server->client_sockets[i] > server->master_socket) {
                max_sd = server->client_sockets[i];
            }
        }

        //wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            printf("select error: %d, errno: %d\n", activity, errno);
        }

        //If something happened on the master socket, then its an incoming connection
        if (FD_ISSET(server->master_socket, &readfds)) {
            server_accept_new_connection(server);
        }

        for (int sock_idx = 0; sock_idx < server->max_clients; sock_idx++) {
            if (FD_ISSET(server->client_sockets[sock_idx], &readfds)) {
                if ((valread = read(server->client_sockets[sock_idx], buffer, MAX_READ_BUFFER_SIZE)) <= 0) {
                    if (valread == -1) {
                        printf("Read error: %i\n", errno);
                    }
                    //Somebody disconnected. Close the socket and mark as 0 in list for reuse
                    server_handle_client_close(server, sock_idx);
                } else {
                    Request *req = malloc(sizeof(Request));
                    req->socket_idx = sock_idx;
                    req->read_buffer = buffer;
                    req->read_buffer_size = valread;
                    server_handle_client_req(server, req);
                    free(req);
                }
            }
        }
    }
    server_free(server);
}

void server_stop(server_t *server) {
    server->stop = 1;
}

