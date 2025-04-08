#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <signal.h>

void handler(int sig_num){
    printf("Received signal: %d\n", sig_num);
}

int main(int argc, char *argv[]){

    if(argc != 2){
      perror("Wrong arg count\n");
      return -1;
    }

    char *signal_reaction = argv[1];
    if(strcmp(signal_reaction, "none") == 0){
        signal(SIGUSR1, SIG_DFL);
    }
    else if(strcmp(signal_reaction, "ignore") == 0){
        signal(SIGUSR1, SIG_IGN);
    }
    else if(strcmp(signal_reaction, "handler") == 0){
        signal(SIGUSR1, handler);
    }
    else if(strcmp(signal_reaction, "mask") == 0){
        sigset_t new_mask;
        sigemptyset(&new_mask);
        sigaddset(&new_mask, SIGUSR1);

        if(sigprocmask(SIG_SETMASK, &new_mask, NULL) < 0){
            perror("Coulnd't set new mask");
            return -1;
        }
    }
    else{
        perror("Unknown reaction");
        return -1;
    }

    raise(SIGUSR1);

    return 0;
}
