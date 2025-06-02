#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 1024

typedef struct {
    char name[32];
    struct sockaddr_in addr;
    int active;
} client_t;

int server_sock;
client_t clients[MAX_CLIENTS];

void list_clients(char *response) {
    strcpy(response, "Active clients:\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            strcat(response, clients[i].name);
            strcat(response, "\n");
        }
    }
}

int find_client_by_name(const char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0)
            return i;
    }
    return -1;
}

void broadcast(const char *msg, const char *from, struct sockaddr_in *exclude) {
    char buffer[BUF_SIZE];
    time_t now = time(NULL);
    snprintf(buffer, sizeof(buffer), "[%s] %s: %s\n", strtok(ctime(&now), "\n"), from, msg);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active &&
            (exclude == NULL || memcmp(&clients[i].addr, exclude, sizeof(struct sockaddr_in)) != 0)) {
            sendto(server_sock, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
        }
    }
}

void handle_message(char *msg, struct sockaddr_in *client_addr) {
    char cmd[8], target[32], content[BUF_SIZE];
    sscanf(msg, "%s", cmd);

    if (strcmp(cmd, "REGISTER") == 0) {
        char name[32];
        sscanf(msg, "REGISTER %s", name);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                strcpy(clients[i].name, name);
                clients[i].addr = *client_addr;
                clients[i].active = 1;
                printf("Registered: %s\n", name);
                break;
            }
        }
    } else if (strcmp(cmd, "LIST") == 0) {
        char response[BUF_SIZE];
        list_clients(response);
        sendto(server_sock, response, strlen(response), 0,
               (struct sockaddr *)client_addr, sizeof(*client_addr));
    } else if (strcmp(cmd, "2ALL") == 0) {
        sscanf(msg, "2ALL %[^\n]", content);
        // find who sent it
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active && memcmp(&clients[i].addr, client_addr, sizeof(*client_addr)) == 0) {
                broadcast(content, clients[i].name, client_addr);
                break;
            }
        }
    } else if (strcmp(cmd, "2ONE") == 0) {
        sscanf(msg, "2ONE %s %[^\n]", target, content);
        int to = find_client_by_name(target);
        if (to >= 0) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active && memcmp(&clients[i].addr, client_addr, sizeof(*client_addr)) == 0) {
                    char buffer[BUF_SIZE];
                    time_t now = time(NULL);
                    snprintf(buffer, sizeof(buffer), "[%s] %s (private): %s\n",
                             ctime(&now), clients[i].name, content);
                    sendto(server_sock, buffer, strlen(buffer), 0,
                           (struct sockaddr *)&clients[to].addr, sizeof(clients[to].addr));
                }
            }
        }
    } else if (strcmp(cmd, "STOP") == 0) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active && memcmp(&clients[i].addr, client_addr, sizeof(*client_addr)) == 0) {
                clients[i].active = 0;
                printf("Client %s disconnected\n", clients[i].name);
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    server_sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("UDP Chat server listening on port %d...\n", port);

    while (1) {
        int len = recvfrom(server_sock, buffer, BUF_SIZE - 1, 0,
                           (struct sockaddr *)&client_addr, &addr_len);
        if (len > 0) {
            buffer[len] = '\0';
            handle_message(buffer, &client_addr);
        }
    }

    close(server_sock);
    return 0;
}