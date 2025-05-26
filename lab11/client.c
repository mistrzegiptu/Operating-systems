#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define BUF_SIZE 1024
#define NAME_LEN 32

int sock;
char name[NAME_LEN];

void *recv_handler(void *arg) {
    char buffer[BUF_SIZE];
    while (1) {
        int len = recv(sock, buffer, BUF_SIZE - 1, 0);
        if (len <= 0) break;
        buffer[len] = '\0';
        printf("%s", buffer);
    }
    return NULL;
}

void handle_exit(int sig) {
    send(sock, "STOP", 4, 0);
    close(sock);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <name> <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    strncpy(name, argv[1], NAME_LEN);
    char *ip = argv[2];
    int port = atoi(argv[3]);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr.s_addr = inet_addr(ip)
    };

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        perror("Connect failed");
        return 1;
    }

    send(sock, name, strlen(name), 0);

    signal(SIGINT, handle_exit);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, recv_handler, NULL);

    char buffer[BUF_SIZE];
    while (fgets(buffer, BUF_SIZE, stdin)) {
        send(sock, buffer, strlen(buffer), 0);
    }

    return 0;
}