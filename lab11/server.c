#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 1024
#define NAME_LEN 32
#define ALIVE_INTERVAL 15

typedef struct {
    int sock;
    char name[NAME_LEN];
    int active;
} Client;

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast(const char *msg, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active && clients[i].sock != sender_sock) {
            send(clients[i].sock, msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_one(const char *msg, const char *target) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active && strcmp(clients[i].name, target) == 0) {
            send(clients[i].sock, msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void list_clients(int sock) {
    char buffer[BUF_SIZE] = "Clients:\n";
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].active) {
            strcat(buffer, clients[i].name);
            strcat(buffer, "\n");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    send(sock, buffer, strlen(buffer), 0);
}

void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].sock == sock) {
            clients[i].active = 0;
            close(clients[i].sock);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *client_handler(void *arg) {
    int sock = *(int *)arg;
    char buffer[BUF_SIZE];
    char name[NAME_LEN] = {0};

    recv(sock, name, NAME_LEN, 0);

    pthread_mutex_lock(&clients_mutex);
    int added = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i].active) {
            clients[i].sock = sock;
            clients[i].active = 1;
            strncpy(clients[i].name, name, NAME_LEN);
            added = 1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (!added) {
        send(sock, "Server full\n", 12, 0);
        close(sock);
        return NULL;
    }

    while (1) {
        int len = recv(sock, buffer, BUF_SIZE, 0);
        if (len <= 0) break;
        buffer[len] = '\0';

        if (strncmp(buffer, "LIST", 4) == 0) {
            list_clients(sock);
        } else if (strncmp(buffer, "2ALL ", 5) == 0) {
            time_t now = time(NULL);
            char msg[BUF_SIZE];
            snprintf(msg, sizeof(msg), "[%s][%s]: %s", name, ctime(&now), buffer + 5);
            broadcast(msg, sock);
        } else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char *target = strtok(buffer + 5, " ");
            char *msg_body = strtok(NULL, "");
            if (target && msg_body) {
                time_t now = time(NULL);
                char msg[BUF_SIZE];
                snprintf(msg, sizeof(msg), "[%s->%s][%s]: %s", name, target, ctime(&now), msg_body);
                send_to_one(msg, target);
            }
        } else if (strncmp(buffer, "STOP", 4) == 0) {
            break;
        }
    }

    remove_client(sock);
    return NULL;
}

void *alive_checker(void *arg) {
    while (1) {
        sleep(ALIVE_INTERVAL);
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].active) {
                if (send(clients[i].sock, "ALIVE?\n", 7, 0) <= 0) {
                    clients[i].active = 0;
                    close(clients[i].sock);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(atoi(argv[1])),
            .sin_addr.s_addr = INADDR_ANY
    };

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CLIENTS);
    printf("Server listening on port %s\n", argv[1]);

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, alive_checker, NULL);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &len);
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, &client_sock);
        pthread_detach(tid);
    }

    return 0;
}