#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

#define FIFO_AB_PATH "/tmp/fifo_ab"
#define FIFO_RES_PATH "/tmp/fifo_result"

void clean_at_exit(){
    unlink(FIFO_AB_PATH);
    unlink(FIFO_RES_PATH);
}

double function(double x) {
    return 4.0 / (1.0 + x * x);
}

double calculate_integral(double a, double b, double width) {
    double sum = 0.0;
    int n = (int)((b - a) / width);
    for (int i = 0; i < n; i++) {
        double x = a + i * width;
        sum += function(x) * width;
    }
    return sum;
}

int main() {
    atexit(clean_at_exit);
    signal(SIGINT, clean_at_exit);
    signal(SIGTERM, clean_at_exit);
    signal(SIGTSTP, clean_at_exit);

    const char* fifo_ab = FIFO_AB_PATH;
    const char* fifo_result = FIFO_RES_PATH;

    mkfifo(fifo_ab, 0666);
    mkfifo(fifo_result, 0666);

    while (1) {
        int fd_read = open(fifo_ab, O_RDONLY);
        double a, b;
        read(fd_read, &a, sizeof(double));
        read(fd_read, &b, sizeof(double));
        close(fd_read);

        double width = 0.000001;
        double result = calculate_integral(a, b, width);

        int fd_write = open(fifo_result, O_WRONLY);
        write(fd_write, &result, sizeof(double));
        close(fd_write);
    }

    return 0;
}
