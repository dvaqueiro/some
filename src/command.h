#ifndef COMMAND_H
#define COMMAND_H

#include <stdlib.h>
#include "server.h"

#define VERSION "0.0.1"
#define PROG_NAME "SoMe"

int process_command(server_t *server, char *command, char *response, size_t response_size);

#endif // COMMAND_H
