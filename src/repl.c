#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.0.1"
#define PROG_NAME "SoMe"
#define WELLCOME PROG_NAME ". Save Objects in Memory.\nDaniel Vaqueiro Crispin.\nType exit to return to cli.\n"

int repl_exit = 0;

void process_command(char *command);

int main(int argc, char **argv) {
    char *command = NULL;
    size_t len = 10;

    printf("%s", WELLCOME);
    while (!repl_exit) {
        printf("%s >> ", PROG_NAME);
        getline(&command, &len, stdin);

        if (!command) {
            break;
        }
        process_command(command);
    }

    printf("Bye!\n");
    free(command);

    return 0;
}

void process_command(char *command) {
    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "exit") == 0) {
        repl_exit = 1;
    } else {
        printf("Command not found.\n");
    }
}
