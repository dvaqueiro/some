#include <stdio.h>
#include <string.h>
#include "command.h"
#include "database.h"
#include "server.h"

char *strdelch(char *str, char ch) {
    char *current = str;
    char *tail = str;

    while(*tail) {
        if(*tail == ch) {
            tail++;
        } else {
            *current++ = *tail++;
        }
    }
    *current = 0;

    return str;
}

void tokenize_command(char *command, char **tokens, size_t *num_tokens) {
    size_t i = 0;
    const char *start;
    int state = ' ';

    while (*command) {
        switch (state) {
            case ' ': // Consuming spaces
                if (*command == '\"') {
                    start = command;
                    state = '\"';  // begin quote
                } else if (*command != ' ') {
                    start = command;
                    state = 'T';
                }
                break;
            case 'T': // non-quoted text
                if (*command == ' ') {
                    tokens[i++] = strndup(start, command - start);
                    state = ' ';
                } else if (*command == '\"') {
                    state = '\"'; // begin quote
                }
                break;
            case '\"': // Inside a quote
                if (*command == '\"') {
                    state = 'T'; // end quote
                }
                break;
        }
        command++;
    }

    if (state != ' ') {
        tokens[i++] = strndup(start, command - start);
    }

    *num_tokens = i;
}

int process_command(server_t *server, char *command, char *response, size_t max_res_buff_size) {
    int res_len = 0;
    size_t num_tokens;
    char *tokens[100];
    database *db = server->db;

    tokenize_command(command, tokens, &num_tokens);

    if (strcmp(tokens[0], "version") == 0) {
        res_len = snprintf(response, max_res_buff_size, "%s Version %s\n",PROG_NAME, VERSION);
    }

    if (strcmp(tokens[0], "stop") == 0) {
        server_stop(server);
        res_len = snprintf(response, max_res_buff_size, "OK\n");
    }

    if (strcmp(tokens[0], "status") == 0) {
        res_len = server_status(server, response, max_res_buff_size);
    }

    if (strcmp(tokens[0], "keys") == 0 && num_tokens == 1) {
        int num_keys = 0;
        char **keys = db_keys(db, &num_keys);
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
        char *val = strdelch(tokens[2], '\"');
        db_insert(db, tokens[1], val);
        res_len = snprintf(response, max_res_buff_size, "OK\n");
    }
    if (strcmp(tokens[0], "get") == 0 && num_tokens == 2) {
        char *val = (char *)db_lookup(db, tokens[1]);
        if (val) {
            res_len = snprintf(response, max_res_buff_size, "\"%s\"\n", val);
        } else {
            res_len = snprintf(response, max_res_buff_size, "(nil)\n");
        }
    }

    if (strcmp(tokens[0], "flushall") == 0 && num_tokens == 1) {
        db_flushall(db);
        res_len = snprintf(response, max_res_buff_size, "OK\n");
    }

    if (res_len == 0) {
        res_len = snprintf(response, max_res_buff_size, "Invalid command\n");
    }

    return res_len;
}
