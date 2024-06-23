#include <stdio.h>
#include "server.h"

#define PORT 8008
#define BACKLOG 5

int main() {
    server_t *server;

    server = server_init(PORT, BACKLOG);
    server_main_loop(server);
    server_free(server);
    printf("Server stopped\n");

    return 0;
}
