#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#define DEFAULT_CAPACITY 6

int total_patients;
int total_pharmacists;
int capacity = DEFAULT_CAPACITY;

volatile int meds;
volatile int waiting_patients = 0;
volatile int served_patients = 0;

volatile int patient_queue[3];
volatile int queue_head = 0, queue_tail = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t doctor_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t patient_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t pharmacist_cv = PTHREAD_COND_INITIALIZER;

void print_time() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d]", t->tm_hour, t->tm_min, t->tm_sec);
}

void* patient_thread(void* arg) {
    int id = (intptr_t)arg;
    while (1) {
        int arrive = rand() % 4 + 2;
        print_time();
        printf(" Pacjent(%d): Ide do szpitala, bede za %d s\n", id, arrive);
        sleep(arrive);

        pthread_mutex_lock(&mutex);
        if (served_patients >= total_patients) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        while (waiting_patients >= 3) {
            int back = rand() % 4 + 2;
            print_time();
            printf(" Pacjent(%d): za duzo pacjentow, wracam pozniej za %d s\n", id, back);
            pthread_mutex_unlock(&mutex);
            sleep(back);
            pthread_mutex_lock(&mutex);
            if (served_patients >= total_patients) {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
        waiting_patients++;
        patient_queue[queue_tail] = id;
        queue_tail = (queue_tail + 1) % 3;
        print_time();
        printf(" Pacjent(%d): czeka %d pacjentow na lekarza\n", id, waiting_patients);
        if (waiting_patients == 3) {
            print_time();
            printf(" Pacjent(%d): budze lekarza\n", id);
            pthread_cond_signal(&doctor_cv);
        }
        pthread_cond_wait(&patient_cv, &mutex);
        print_time();
        printf(" Pacjent(%d): koncze wizyte\n", id);
        pthread_mutex_unlock(&mutex);
        break;
    }
    return NULL;
}

void* pharmacist_thread(void* arg) {
    int id = (intptr_t)arg;
    int arrive = rand() % 11 + 5;
    print_time();
    printf(" Farmaceuta(%d): ide do szpitala, bede za %d s\n", id, arrive);
    sleep(arrive);

    pthread_mutex_lock(&mutex);
    while (meds == capacity) {
        print_time();
        printf(" Farmaceuta(%d): czekam na oproznienie apteczki\n", id);
        pthread_cond_wait(&pharmacist_cv, &mutex);
    }
    if (meds < 3) {
        print_time();
        printf(" Farmaceuta(%d): budze lekarza\n", id);
        pthread_cond_signal(&doctor_cv);
    }
    print_time();
    printf(" Farmaceuta(%d): dostarczam leki\n", id);
    pthread_cond_signal(&doctor_cv);
    pthread_mutex_unlock(&mutex);

    int deliver = rand() % 3 + 1;
    sleep(deliver);

    pthread_mutex_lock(&mutex);
    print_time();
    printf(" Farmaceuta(%d): zakonczylem dostaw\n", id);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* doctor_thread(void* arg) {
    (void)arg;
    pthread_mutex_lock(&mutex);
    while (served_patients < total_patients) {
        while (!((waiting_patients >= 3 && meds >= 3) || (meds < 3) || (meds < capacity))) {
            pthread_cond_wait(&doctor_cv, &mutex);
        }
        print_time();
        printf(" Lekarz: budze sie\n");

        if (waiting_patients >= 3 && meds >= 3) {
            int ids[3];
            for (int i = 0; i < 3; i++) {
                ids[i] = patient_queue[queue_head];
                queue_head = (queue_head + 1) % 3;
            }
            print_time();
            printf(" Lekarz: konsultuje pacjentow %d, %d, %d\n", ids[0], ids[1], ids[2]);
            meds -= 3;
            waiting_patients -= 3;
            served_patients += 3;

            pthread_mutex_unlock(&mutex);
            int consult = rand() % 3 + 2;
            sleep(consult);
            pthread_mutex_lock(&mutex);

            pthread_cond_broadcast(&patient_cv);

        } else if (meds < 3 || meds < capacity) {
            print_time();
            printf(" Lekarz: przyjmuje dostawę lekow\n");
            pthread_mutex_unlock(&mutex);
            int receive = rand() % 3 + 1;
            sleep(receive);
            pthread_mutex_lock(&mutex);
            meds = capacity;
        }

        print_time();
        printf(" Lekarz: zasypiam\n");
        pthread_cond_broadcast(&pharmacist_cv);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_patients> <num_pharmacists>\n", argv[0]);
        return EXIT_FAILURE;
    }
    total_patients = atoi(argv[1]);
    total_pharmacists = atoi(argv[2]);
    if (total_patients % 3 != 0) {
        fprintf(stderr, "Warning: total_patients not multiple of 3, some may never be consulted.\n");
    }
    meds = capacity;
    srand(time(NULL));

    pthread_t doc;
    pthread_t* pats = malloc(sizeof(pthread_t) * total_patients);
    pthread_t* pharms = malloc(sizeof(pthread_t) * total_pharmacists);

    pthread_create(&doc, NULL, doctor_thread, NULL);

    for (int i = 0; i < total_patients; i++) {
        pthread_create(&pats[i], NULL, patient_thread, (void*)(intptr_t)(i+1));
    }

    for (int j = 0; j < total_pharmacists; j++) {
        pthread_create(&pharms[j], NULL, pharmacist_thread, (void*)(intptr_t)(j+1));
    }

    for (int i = 0; i < total_patients; i++) {
        pthread_join(pats[i], NULL);
    }
    for (int j = 0; j < total_pharmacists; j++) {
        pthread_join(pharms[j], NULL);
    }
    pthread_join(doc, NULL);

    free(pats);
    free(pharms);
    return 0;
}
