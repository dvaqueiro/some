#include <stdio.h>
#include <string.h>
#include "command.h"
#include "hashtable.h"

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

void process_command(server_t *server, char *command, char *response, size_t response_size) {
    size_t num_tokens;
    char *tokens[100];
    hash_table *table = server->table;

    tokenize_command(command, tokens, &num_tokens);

    if (strcmp(tokens[0], "version") == 0) {
        snprintf(response, response_size, "%s Version %s\n",PROG_NAME, VERSION);
    }

    if (strcmp(tokens[0], "stop") == 0) {
        server->stop = 1;
    }

    if (strcmp(tokens[0], "print") == 0) {
        hash_table_print(table);
        snprintf(response, response_size, "OK\n");
    }

    if (strcmp(tokens[0], "set") == 0 && num_tokens == 3) {
        hash_table_insert(table, tokens[1], tokens[2]);
        snprintf(response, response_size, "OK\n");
    }
    if (strcmp(tokens[0], "get") == 0 && num_tokens == 2) {
        char *val = (char *)hash_table_lookup(table, tokens[1]);
        if (val) {
            snprintf(response, response_size, "\"%s\"\n", val);
        } else {
            snprintf(response, response_size, "(nil)\n");
        }
    }
}
