#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

#define FIFO_AB_PATH "/tmp/fifo_ab"
#define FIFO_RES_PATH "/tmp/fifo_result"


int main() {
    const char* fifo_ab = FIFO_AB_PATH;
    const char* fifo_result = FIFO_RES_PATH;

    mkfifo(fifo_ab, 0666);
    mkfifo(fifo_result, 0666);

    double a, b;
    printf("Input interval [a b]: ");
    scanf("%lf %lf", &a, &b);

    int fd_write = open(fifo_ab, O_WRONLY);
    write(fd_write, &a, sizeof(double));
    write(fd_write, &b, sizeof(double));
    close(fd_write);

    int fd_read = open(fifo_result, O_RDONLY);
    double result = 0.0;
    read(fd_read, &result, sizeof(double));
    close(fd_read);

    printf("Result in [%.4lf, %.4lf] is: %.15lf\n", a, b, result);

    return 0;
}
