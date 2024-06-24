#include <stdio.h>
#include "server.h"

#define PORT 8008
#define BACKLOG 5

int main() {
    server_t *server;

    server = server_new(PORT, BACKLOG);
    server_run(server);
    printf("Server gracefull stopped\n");

    return 0;
}
