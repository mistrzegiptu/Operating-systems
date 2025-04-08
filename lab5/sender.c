#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void rec_conf(int sig_num){
    printf("Received signal: %d \n", sig_num);
}

int main(int argc, char *argv[]){
    if(argc != 3){
        perror("Wrong argument count\n");
        return 0;
    }

    int pid = atoi(argv[1]);
    int mode = atoi(argv[2]);
    union sigval sigval_mode = {.sival_int = mode};

    sigqueue(pid, SIGUSR1, sigval_mode);

    struct sigaction action = {.sa_flags = 0, .sa_handler = rec_conf};
    sigemptyset(&action.sa_mask);

    sigaction(SIGUSR1, &action, NULL);
    //pause();

    return 0;
}