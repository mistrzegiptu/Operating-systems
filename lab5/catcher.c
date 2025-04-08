#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t mode = 0;
volatile sig_atomic_t counter = 0;
volatile sig_atomic_t is_counting = 0;

void sigint_handler(){
    printf("Pressed CTR + C\n");
}

void catcher(int sig_num, siginfo_t *info, void *ucontext){

    int new_mode = info->si_value.sival_int;

    if(new_mode != mode){
        mode = new_mode;
        counter++;
    }

    kill(info->si_pid, SIGUSR1);
    is_counting = 0;

    switch (mode)
    {
        case 1:
        {
            printf("Catcher switched modes %d times\n", counter);
            break;
        }
        case 2:
        {
            is_counting = 1;
            break;
        }
        case 3:
        {
            signal(SIGINT, SIG_IGN);
            break;
        }
        case 4:
        {
            signal(SIGINT, sigint_handler);
            break;
        }
        case 5:
        {
            exit(0);
        }
        default:
        {
            printf("Unknown signal\n");
            break;
        }
    }

}

int main(){

    printf("Catcher's PID: %d\n", getpid());

    struct sigaction action;
    action.sa_sigaction = catcher;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR1, &action, NULL);

    int i = 0;
    while(1) {

        while(is_counting == 1){
            printf("%d\n", i);
            i++;
            sleep(1);
        }
        pause();
    }

    return 0;
}