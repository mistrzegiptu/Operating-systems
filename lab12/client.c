#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int sock;
struct sockaddr_in server_addr;
char client_name[32];

void *recv_thread(void *arg) {
    char buffer[BUF_SIZE];
    while (1) {
        int len = recvfrom(sock, buffer, BUF_SIZE - 1, 0, NULL, NULL);
        if (len > 0) {
            buffer[len] = '\0';
            printf("%s", buffer);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <name> <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    strcpy(client_name, argv[1]);
    char *server_ip = argv[2];
    int port = atoi(argv[3]);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    char reg_msg[BUF_SIZE];
    snprintf(reg_msg, sizeof(reg_msg), "REGISTER %s", client_name);
    sendto(sock, reg_msg, strlen(reg_msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    pthread_t tid;
    pthread_create(&tid, NULL, recv_thread, NULL);

    char input[BUF_SIZE];
    while (fgets(input, sizeof(input), stdin)) {
        sendto(sock, input, strlen(input), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (strncmp(input, "STOP", 4) == 0) break;
    }

    close(sock);
    return 0;
}
