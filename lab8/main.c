#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>

#define QUEUE_SIZE 10
#define JOB_LENGTH 10
#define SHM_NAME "/print_queue"
#define SEM_MUTEX "/sem_mutex"
#define SEM_FULL  "/sem_full"
#define SEM_EMPTY "/sem_empty"
#define N_USERS 3
#define M_PRINTERS 2

typedef struct {
    char jobs[QUEUE_SIZE][JOB_LENGTH + 1];
    int front;
    int rear;
} job_queue_t;

job_queue_t *queue;
sem_t *mutex, *full, *empty;
int shm_fd;

void cleanup(int signum) {
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_EMPTY);

    munmap(queue, sizeof(job_queue_t));
    close(shm_fd);
    shm_unlink(SHM_NAME);

    exit(0);
}

void enqueue(char *job) {
    strncpy(queue->jobs[queue->rear], job, JOB_LENGTH + 1);
    queue->rear = (queue->rear + 1) % QUEUE_SIZE;
}

void dequeue(char *dest) {
    strncpy(dest, queue->jobs[queue->front], JOB_LENGTH + 1);
    queue->front = (queue->front + 1) % QUEUE_SIZE;
}

void *user_thread(void *arg) {
    int id = *((int *)arg);
    char job[JOB_LENGTH + 1];

    while (1) {
        for (int i = 0; i < JOB_LENGTH; ++i) {
            job[i] = 'a' + rand() % 26;
        }
        job[JOB_LENGTH] = '\0';

        sem_wait(empty);
        sem_wait(mutex);

        enqueue(job);
        printf("\nUżytkownik %d dodał zadanie: %s\n", id, job);

        sem_post(mutex);
        sem_post(full);

        sleep(rand() % 4 + 1);
    }
    return NULL;
}

void *printer_thread(void *arg) {
    int id = *((int *)arg);
    char job[JOB_LENGTH + 1];

    while (1) {
        sem_wait(full);
        sem_wait(mutex);

        dequeue(job);

        sem_post(mutex);
        sem_post(empty);

        printf("\nDrukarka %d drukuje: ", id);
        fflush(stdout);

        for (int i = 0; i < JOB_LENGTH; ++i) {
            printf("%c ", job[i]);
            fflush(stdout);
            sleep(1);
        }
        printf("\n");
    }
    return NULL;
}

int main() {
    signal(SIGINT, cleanup);
    srand(time(NULL));

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(job_queue_t));
    queue = mmap(0, sizeof(job_queue_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    queue->front = 0;
    queue->rear = 0;

    mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);
    empty = sem_open(SEM_EMPTY, O_CREAT, 0666, QUEUE_SIZE);

    pthread_t users[N_USERS], printers[M_PRINTERS];
    int ids[N_USERS > M_PRINTERS ? N_USERS : M_PRINTERS];

    for (int i = 0; i < N_USERS; ++i) {
        ids[i] = i;
        pthread_create(&users[i], NULL, user_thread, &ids[i]);
    }

    for (int i = 0; i < M_PRINTERS; ++i) {
        ids[i] = i;
        pthread_create(&printers[i], NULL, printer_thread, &ids[i]);
    }

    for (int i = 0; i < N_USERS; ++i)
        pthread_join(users[i], NULL);
    for (int i = 0; i < M_PRINTERS; ++i)
        pthread_join(printers[i], NULL);

    return 0;
}
