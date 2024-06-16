#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "hashtable.h"

#define VERSION "0.0.1"
#define PROG_NAME "SoMe"
#define WELLCOME PROG_NAME ". Save Objects in Memory.\nDaniel Vaqueiro Crispin.\nType exit to return to cli.\n"

#define FNV_PRIME 0x10000001b3
#define FNV_OFFSET 0xcbf29ce48422325UL

void process_command(char *command, hash_table *table);
void tokenize_command(char *command, char **tokens, size_t *num_tokens);
uint64_t hash_fnv1(const char *key, size_t length);

int main(int argc, char **argv) {
    char *command = NULL;
    size_t len = 10;

    // 1<<20 == 1048576 (1 MegaEntry)
    //const int tablesize = (1<<20);
    const int tablesize = 5;
    hash_table *table = hash_table_create(tablesize, hash_fnv1, NULL);

    printf("%s", WELLCOME);
    while (true) {
        printf("%s >> ", PROG_NAME);
        getline(&command, &len, stdin);

        if (!command) {
            break;
        }
        process_command(command, table);
    }

    free(command);
    hash_table_destroy(table);

    return 0;
}

void process_command(char *command, hash_table *table) {
    size_t num_tokens;
    char *tokens[100];

    command[strcspn(command, "\n")] = '\0';

    tokenize_command(command, tokens, &num_tokens);

    if (strcmp(tokens[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(tokens[0], "echo") == 0) {
        for (size_t i = 1; i < num_tokens; i++) {
            printf("Token %zu: %s\n", i, tokens[i]);
        }
    }

    if (strcmp(tokens[0], "version") == 0) {
        printf("%s Version %s\n",PROG_NAME, VERSION);
    }

    if (strcmp(tokens[0], "print") == 0) {
        hash_table_print(table);
    }

    if (strcmp(tokens[0], "set") == 0 && num_tokens == 3) {
        hash_table_insert(table, tokens[1], tokens[2]);
        printf("OK\n");
    }
    if (strcmp(tokens[0], "get") == 0 && num_tokens == 2) {
        char *val = (char *)hash_table_lookup(table, tokens[1]);
        if (val) {
            printf("\"%s\"\n", val);
        } else {
            printf("(nil)\n");
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
