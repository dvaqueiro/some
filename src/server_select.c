#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "hashtable.h"

#define VERSION "0.0.1"
#define PROG_NAME "SoMe"

#define PORT 8008
#define SOCKET_ERROR -1
#define MAX_CLIENTS 2
#define BACKLOG 5

#define FNV_PRIME 0x10000001b3
#define FNV_OFFSET 0xcbf29ce48422325UL

typedef struct {
    struct sockaddr_in addr;
    int master_socket;
    int client_sockets[MAX_CLIENTS];
} server_t;

server_t *server_init(short port, int backlog);
void server_main_loop(server_t *server, hash_table *table);
void server_accept_new_connection(server_t *server, char *welcome);
void server_handle_client_close(server_t *server, int client_socket_idx);
void server_handle_client_req(server_t *server, hash_table *table, int client_socket_idx, char *buffer, int valread);
void server_print_sockets(server_t *server);

int check_or_exit(int err, const char *msg);

void process_command(hash_table *table, char *command, char *response);
void tokenize_command(char *command, char **tokens, size_t *num_tokens);
uint64_t hash_fnv1(const char *key, size_t length);

int main() {
    const int tablesize = 5;
    hash_table *table = hash_table_create(tablesize, hash_fnv1, NULL);
    server_t *server;

    server = server_init(PORT, BACKLOG);
    server_main_loop(server, table);

    return 0;
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
server_t *server_init(short port, int backlog) {
    server_t *server = malloc(sizeof(server_t));
    if (server == NULL) {
        perror("Failed to allocate memory for server");
        exit(1);
    }

    server->master_socket = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
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
    printf("Waiting for connections on 0.0.0.0:%d\n", PORT);

    return server;
}

void server_print_sockets(server_t *server) {
    printf("ip: %s, port: %d\n", inet_ntoa(server->addr.sin_addr), ntohs(server->addr.sin_port));
    printf("[Master socket]: %d\n", server->master_socket);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        printf("[%d] -> %d\n", i, server->client_sockets[i]);
    }
}

int check_or_exit(int err, const char *msg) {
    if (err == SOCKET_ERROR) {
        perror(msg);
        exit(1);
    }
    return err;
}

void server_accept_new_connection(server_t *server, char *welcome) {
    int new_socket, new_con_accepted = 0;

    if ((new_socket = accept(server->master_socket, (struct sockaddr *)&server->addr, (socklen_t*)&server->addr)) == SOCKET_ERROR) {
        perror("accept");
        printf("Failed accepting connection\n");
    }
    //add new socket to array of sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->client_sockets[i] == 0) {
            server->client_sockets[i] = new_socket;
            server_print_sockets(server);
            new_con_accepted = 1;
            send(new_socket, welcome, strlen(welcome), 0);
            printf(
                    "Connection accepted (socket: %d) from ip:%s port:%d\n",
                    new_socket,
                    inet_ntoa(server->addr.sin_addr),
                    ntohs(server->addr.sin_port)
                  );
            break;
        }
    }
    if (new_con_accepted == 0) {
        printf("No more connections allowed\n");
        close(new_socket);
    }
}

void server_handle_client_close(server_t *server, int client_socket_idx) {
    getpeername(server->client_sockets[client_socket_idx], (struct sockaddr*)&server->addr, (socklen_t*)&server->addr);
    printf(
            "Host disconnected (socked: %d), ip %s, port %d\n",
            server->client_sockets[client_socket_idx],
            inet_ntoa(server->addr.sin_addr),
            ntohs(server->addr.sin_port)
          );
    close(server->client_sockets[client_socket_idx]);
    server->client_sockets[client_socket_idx] = 0;
}

void server_handle_client_req(server_t *server, hash_table *table, int client_socket_idx, char *buffer, int valread) {
    char response[1024] = "";
    //buffer[valread] = '\0';
    buffer[strcspn(buffer, "\r\n")] = '\0';
    process_command(table, buffer, response);
    send(server->client_sockets[client_socket_idx], response, strlen(response), 0);
}

void server_main_loop(server_t *server, hash_table *table) {
    int max_sd, activity, valread;
    fd_set readfds;
    char buffer[1025];
    char *welcome = "Welcome to the server\n";

    while (1) {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(server->master_socket, &readfds);

        //add child sockets to set
        max_sd = server->master_socket;
        for (int i = 0; i < MAX_CLIENTS; i++) {
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
            server_accept_new_connection(server, welcome);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (FD_ISSET(server->client_sockets[i], &readfds)) {
                if ((valread = read(server->client_sockets[i], buffer, 1024)) == 0) {
                    //Somebody disconnected. Close the socket and mark as 0 in list for reuse
                    server_handle_client_close(server, i);
                } else {
                    server_handle_client_req(server, table, i, buffer, valread);
                }
            }
        }
    }
}

void process_command(hash_table *table, char *command, char *response) {
    size_t num_tokens;
    char *tokens[100];

    tokenize_command(command, tokens, &num_tokens);

    if (strcmp(tokens[0], "version") == 0) {
        sprintf(response, "%s Version %s\n",PROG_NAME, VERSION);
    }

    if (strcmp(tokens[0], "print") == 0) {
        hash_table_print(table);
        sprintf(response, "OK\n");
    }

    if (strcmp(tokens[0], "set") == 0 && num_tokens == 3) {
        hash_table_insert(table, tokens[1], tokens[2]);
        sprintf(response, "OK\n");
    }
    if (strcmp(tokens[0], "get") == 0 && num_tokens == 2) {
        char *val = (char *)hash_table_lookup(table, tokens[1]);
        if (val) {
            sprintf(response, "\"%s\"\n", val);
        } else {
            sprintf(response, "(nil)\n");
        }
    }
}

void tokenize_command(char *command, char **tokens, size_t *num_tokens) {
    char *token = strtok(command, " ");
    size_t i = 0;

    while (token) {
        if (i >= 100) {
            break;
        }
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }

    *num_tokens = i;
}

uint64_t hash_fnv1(const char *key, size_t length) {
    uint64_t hash_value = FNV_OFFSET;
    for (int i = 0; i < length; i++) {
        hash_value ^= key[i];
        hash_value *= FNV_PRIME;
    }
    return hash_value;
}
