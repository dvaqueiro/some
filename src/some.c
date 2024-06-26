#include <stdio.h>
#include <unistd.h>
#include "server.h"

#define BACKLOG 5

int main(int argc, char **argv) {
    server_t *server;

    int opt, port = 8008;

    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port]\n", argv[0]);
                exit(1);
        }
    }

    server = server_new(port, BACKLOG);
    server_run(server);
    printf("Server gracefull stopped\n");

    return 0;
}
