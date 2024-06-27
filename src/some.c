#include <stdio.h>
#include <unistd.h>
#include "server.h"

#define BACKLOG 2

int main(int argc, char **argv) {
    server_t *server;

    int opt, port = 8008, max_conn = 2;

    while ((opt = getopt(argc, argv, "p:m:v")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                max_conn = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-m max_connections]\n", argv[0]);
                exit(1);
        }
    }

    server = server_new(port, BACKLOG, max_conn);
    server_run(server);
    printf("Server gracefull stopped\n");

    return 0;
}
