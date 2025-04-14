#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

double function(double x){
    return 4.0 / (x * x + 1.0);
}

int main(int argc, char *argv[]){
    if(argc != 3){
        perror("Wrong arg count\n");
        return -1;
    }

    double width = atof(argv[1]);
    int procCount = atoi(argv[2]);
    int rectCount = (int)(1.0 / width);

    if (width <= 0.0 || width > 1.0 || procCount < 1) {
        perror("Error: wrong args\n");
        return -1;
    }

    for(int i = 1; i <= procCount; i++){
        int pipes[i][2];
        pid_t pids[i];

        clock_t start = clock();

        for(int j = 0; j < i; j++){

            if(pipe(pipes[j]) == -1){
                perror("Error while pipeing\n");
                exit(EXIT_FAILURE);
            }

            pids[j] = fork();
            if(pids[j] < 0){
                perror("Error while forking\n");
                return -1;
            }
            else if(pids[j] == 0){
                close(pipes[j][0]);

                double childSum = 0.0;
                for(int k = j; k < rectCount; k += i){
                    double x = k * width;
                    childSum += function(x) * width;
                }

                write(pipes[j][1], &childSum, sizeof(double));
                close(pipes[j][1]);
                exit(EXIT_SUCCESS);
            }
            close(pipes[j][1]);
        }

        double result = 0.0;
        for(int j = 0; j < i; j++){
            double childReturned = 0.0;
            read(pipes[j][0], &childReturned, sizeof(double));
            result += childReturned;
            close(pipes[j][0]);
            waitpid(pids[j], NULL, 0);
        }

        clock_t end = clock();
        printf("%d processes, result: %f , elapsed time: %f s\n", i, result, (float)(end-start)/1000);
    }

    return 0;
}