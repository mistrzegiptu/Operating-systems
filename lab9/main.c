#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 1000

double f(double x) {
    return 4.0 / (x * x + 1);
}

typedef struct {
    int id;
    int num_threads;
    double width;
    int num_rects;
    double *results;
} ThreadData;

void *compute_integral(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int id = data->id;
    int num_threads = data->num_threads;
    double width = data->width;
    int num_rects = data->num_rects;
    double local_sum = 0.0;

    for (int i = id; i < num_rects; i += num_threads) {
        double x = i * width + width / 2.0;
        local_sum += f(x) * width;
    }

    data->results[id] = local_sum;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("Wrong arg count\n");
        return 1;
    }

    double width = atof(argv[1]);
    int num_threads = atoi(argv[2]);

    if (num_threads <= 0 || num_threads > MAX_THREADS || width <= 0 || width > 1) {
        perror("Wrong arg values\n");
        return 1;
    }

    int num_rects = (int)(1.0 / width);

    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    double results[MAX_THREADS] = {0.0};

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].width = width;
        thread_data[i].num_rects = num_rects;
        thread_data[i].results = results;

        pthread_create(&threads[i], NULL, compute_integral, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double total = 0.0;
    for (int i = 0; i < num_threads; i++) {
        total += results[i];
    }

    printf("Integral result: %.15f\n", total);
    return 0;
}
