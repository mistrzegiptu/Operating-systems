#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <limits.h>

#define SERVER_PATH "/chat_queue"
#define MAX_MESSAGE_LEN 1024

int main() {
    int id = getpid();
    int length = snprintf(NULL, 0, "%d", id);
    char *client_id = malloc(sizeof(char) * (length + 1));
    snprintf(client_id, length + 1, "%d", id);

    if(!client_id){
        perror("Error while mallocing\n");
    }

    char client_path[100] = {0};
    strcpy(client_path, SERVER_PATH);
    strcat(client_path, client_id);

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MESSAGE_LEN;

    mqd_t server_queue = mq_open(SERVER_PATH, O_WRONLY);
    mqd_t client_queue = mq_open(client_path, O_RDONLY|O_CREAT, 0666, &attr);
    if(server_queue == -1 || client_queue == -1){
        perror("Error while creating queue");
        return -1;
    }


    char init_message[120] = {0};
    strcat(init_message, "INIT:");
    strcat(init_message, client_path);
    mq_send(server_queue, init_message, strlen(init_message) + 1, 0);

    char message_buffer[MAX_MESSAGE_LEN];
    char message_buffer_with_id[MAX_MESSAGE_LEN+sizeof(int)+1];
    mq_receive(client_queue, message_buffer, MAX_MESSAGE_LEN, NULL);
    int real_id = atoi(message_buffer);
    printf("Client connected as %d\n", real_id);

    pid_t fork_pid = fork();
    if(fork_pid < 0){
        perror("Error while forking\n");
    }
    else if(fork_pid == 0){
        while(1){
            if (mq_receive(client_queue, message_buffer, MAX_MESSAGE_LEN, NULL) > 0) {
                printf("%s\n", message_buffer);
            }
        }
    }

    printf("Enter messages: (exit for quitting)\n");

    while(1){
        memset(message_buffer, 0, MAX_MESSAGE_LEN);
        fgets(message_buffer, MAX_MESSAGE_LEN, stdin);

        message_buffer[strcspn(message_buffer, "\n")] = 0;
        if(strcmp(message_buffer, "exit") == 0){
            break;
        }

        sprintf(message_buffer_with_id, "%d:%s", real_id, message_buffer);
        mq_send(server_queue, message_buffer_with_id, strlen(message_buffer_with_id) + 1, 0);
    }

    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(client_path);
    free(client_id);
    return 0;
}