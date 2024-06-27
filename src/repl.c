#include <arpa/inet.h>
#include <bits/getopt_core.h>
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
    int sockfd, opt, port = 8008;
    char *host = "127.0.0.1";

    while ((opt = getopt(argc, argv, "h:p:v")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'v':
                printf("%s %s\n", PROG_NAME, VERSION);
                exit(0);
            default:
                fprintf(stderr, "Usage: %s [-h host] [-p port]\n", argv[0]);
                exit(1);
        }
    }

    sockfd = connect_init(host, port);
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
    int len;
    int command_len = strlen(command);
    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "exit") == 0) {
        repl_exit = 1;
    } else {
        len = write(sockfd, command, command_len);
        if (len < 1) {
            printf("write");
            exit(1);
        }
        bzero(buff, command_len);
        bzero(buff, MAX_BUFF);
        len = read(sockfd, buff, MAX_BUFF);
        if (len < 1) {
            printf("read");
            exit(1);
        }
        printf("%s", buff);
    }
}
