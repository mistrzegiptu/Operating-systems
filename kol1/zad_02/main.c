#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sighandler(int sig_num, siginfo_t *info, void *ucontext){
    int value = info->si_value.sival_int;
    printf("Value %d\n", value);
}

int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR1, &action, NULL);


    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob aby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigdelset(&new_mask, SIGUSR1);
        action.sa_mask = new_mask;

        sigaction(SIGUSR1, &action, NULL);

        if(sigprocmask(SIG_SETMASK, &new_mask, NULL) < 0){
            perror("Coulnd't set new mask\n");
            return -1;
        }

        pause();
    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        int signal_val = atoi(argv[1]);
        int signal_num = atoi(argv[2]);
        union sigval sigval_value = {.sival_int = signal_val};

        if(sigqueue(child, signal_num, sigval_value) < 0){
            perror("Error while sending signal\n");
            return -1;
        }
    }

    return 0;
}
