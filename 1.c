#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

long total_trials;
long total_hits = 0;
int nthreads;
pthread_mutex_t mutex;

typedef struct {
    long trials_per_thread;
} ThreadData;

void* monte_carlo_pi(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long trials_per_thread = data->trials_per_thread;

    unsigned int seed = time(NULL) ^ pthread_self();
    long local_hits = 0;

    for (long i = 0; i < trials_per_thread; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
        double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

        if (x * x + y * y <= 1.0) {
            local_hits++;
        }
    }

    pthread_mutex_lock(&mutex);
    total_hits += local_hits;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s nthreads ntrials\n", argv[0]);
        return 1;
    }

    nthreads = strtol(argv[1], NULL, 10);
    total_trials = strtol(argv[2], NULL, 10);

    pthread_t* threads = malloc(nthreads * sizeof(pthread_t));
    ThreadData* thread_data = malloc(nthreads * sizeof(ThreadData));
    long trials_per_thread = total_trials / nthreads;

    pthread_mutex_init(&mutex, NULL);

    clock_t start = clock();

    for (int i = 0; i < nthreads; i++) {
        thread_data[i].trials_per_thread = trials_per_thread;
        pthread_create(&threads[i], NULL, monte_carlo_pi, &thread_data[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    double pi = 4.0 * (double)total_hits / (double)total_trials;
    printf("Estimated value of pi: %lf\n", pi);

    clock_t end = clock();
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC; // Время в секундах
    printf("Time taken: %lf seconds\n", elapsed_time);

    pthread_mutex_destroy(&mutex);
    free(threads);
    free(thread_data);

    return 0;
}
