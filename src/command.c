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

int process_command(server_t *server, char *command, char *response, size_t max_res_buff_size) {
    int res_len = 0;
    size_t num_tokens;
    char *tokens[100];
    hash_table *table = server->table;

    tokenize_command(command, tokens, &num_tokens);

    if (strcmp(tokens[0], "version") == 0) {
        res_len = snprintf(response, max_res_buff_size, "%s Version %s\n",PROG_NAME, VERSION);
    }

    if (strcmp(tokens[0], "stop") == 0) {
        server->stop = 1;
    }

    if (strcmp(tokens[0], "print") == 0) {
        printf("##############\n");
        hash_table_print(table);
        printf("##############\n");
        res_len = snprintf(response, max_res_buff_size, "OK\n");
    }
    if (strcmp(tokens[0], "keys") == 0 && num_tokens == 1) {
        int num_keys = 0;
        char **keys = hash_table_keys(table, &num_keys);
        if (num_keys == 0) {
            res_len = snprintf(response, max_res_buff_size, "(empty list)\n");
        } else {
            for (size_t i = 0; i < num_keys; i++) {
                res_len += snprintf(response + res_len, max_res_buff_size - res_len, "%lu) \"%s\"\n",i, keys[i]);
            }
        }
        free(keys);
    }

    if (strcmp(tokens[0], "set") == 0 && num_tokens == 3) {
        hash_table_insert(table, tokens[1], tokens[2]);
        res_len = snprintf(response, max_res_buff_size, "OK\n");
    }
    if (strcmp(tokens[0], "get") == 0 && num_tokens == 2) {
        char *val = (char *)hash_table_lookup(table, tokens[1]);
        if (val) {
            res_len = snprintf(response, max_res_buff_size, "\"%s\"\n", val);
        } else {
            res_len = snprintf(response, max_res_buff_size, "(nil)\n");
        }
    }

    return res_len;
}
