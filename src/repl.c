#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define VERSION "0.0.1"
#define PROG_NAME "SoMe"
#define WELLCOME PROG_NAME ". Save Objects in Memory.\nType exit to return to cli.\n"
#define MAX_BUFF 1024

int repl_exit = 0;

int connect_init(char *host, int port);
void repl_run(int sockfd);
void process_command(char *command, int sockfd);

int main(int argc, char **argv) {
    int sockfd;

    sockfd = connect_init("127.0.0.1", 8008);
    repl_run(sockfd);
    close(sockfd);
    printf("Bye!\n");

    return 0;
}

int connect_init(char *host, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        perror("connect");
        exit(1);
    }
    printf("%s", WELLCOME);

    return sockfd;
}

void repl_run(int sockfd) {
    char *command = NULL;
    size_t len = 0;

    while (!repl_exit) {
        printf("%s >> ", PROG_NAME);
        getline(&command, &len, stdin);
        if (!command) {
            break;
        }
        process_command(command, sockfd);
    }

    free(command);
}

void process_command(char *command, int sockfd) {
    char buff[MAX_BUFF];
    int command_len = strlen(command);
    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "exit") == 0) {
        repl_exit = 1;
    } else {
        write(sockfd, command, command_len);
        bzero(buff, command_len);
        bzero(buff, MAX_BUFF);
        read(sockfd, buff, MAX_BUFF);
        printf("%s", buff);
    }
}
