#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>

#define QUEUE_PATH "/chat_queue"

#define CLIENTS_MAX_COUNT 10
#define MAX_MESSAGE_LEN 1024

typedef struct{
    int client_id;
    mqd_t client_queue;
} Client;

int main() {
    Client clients[CLIENTS_MAX_COUNT];
    int last_client = 0;

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MESSAGE_LEN;

    mqd_t server_queue = mq_open(QUEUE_PATH, O_RDONLY|O_CREAT, 0666, &attr);
    if(server_queue == -1){
        perror("Error while creating queue");
        return -1;
    }


    char message_buffer[MAX_MESSAGE_LEN];
    while(1){
        if(mq_receive(server_queue, message_buffer, MAX_MESSAGE_LEN, NULL) == -1){
            perror("Error while receiving\n");
            break;
        }

        if(strncmp(message_buffer, "INIT", 4) == 0 && last_client < CLIENTS_MAX_COUNT){
            char client_queue_path[MAX_MESSAGE_LEN] = {0};
            sscanf(message_buffer + 5, "%s", client_queue_path);

            mqd_t client_queue = mq_open(client_queue_path, O_WRONLY);
            if(client_queue == -1){
                perror("Error while opening client queue\n");
                break;
            }

            clients[last_client].client_id = last_client + 1;
            clients[last_client].client_queue = client_queue;

            snprintf(message_buffer, MAX_MESSAGE_LEN,"%d", last_client);
            mq_send(client_queue, message_buffer, strlen(message_buffer) + 1, 0);

            printf("Connected client with id %d\n", last_client);

            last_client++;
        }
        else{
            int client_id;
            char message[MAX_MESSAGE_LEN] = {0};
            sscanf(message_buffer, "%d:%s", &client_id, message);

            for(int i = 0; i < last_client; i++){
                if(i != client_id){
                    mqd_t current_queue = clients[i].client_queue;
                    mq_send(current_queue, message, strlen(message) + 1, 0);
                }
            }
        }
    }

    for (int i = 0; i < last_client; i++) {
        mq_close(clients[i].client_queue);
    }

    mq_close(server_queue);
    mq_unlink(QUEUE_PATH);
    return 0;
}