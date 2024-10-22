#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

int nthreads;
int npoints;
double min_x = -2.0;
double max_x = 1.0;
double min_y = -1.5;
double max_y = 1.5;

typedef struct {
    int id;
    int npoints;
    int nthreads;
    double min_x;
    double max_x;
    double min_y;
    double max_y;
    FILE *file;
    pthread_mutex_t *file_mutex;
} ThreadData;

int mandelbrot(double complex c) {
    double complex z = 0;
    for (int i = 0; i < 1000; ++i) {
        z = z * z + c;
        if (cabs(z) >= 2.0) {
            return 0;
        }
    }
    return 1;
}

void* compute_mandelbrot(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int points_per_thread = data->npoints / data->nthreads;
    double dx = (data->max_x - data->min_x) / data->npoints;
    double dy = (data->max_y - data->min_y) / data->npoints;

    for (int i = data->id * points_per_thread; i < (data->id + 1) * points_per_thread; ++i) {
        for (int j = 0; j < data->npoints; ++j) {
            double complex c = data->min_x + i * dx + (data->min_y + j * dy) * I;
            if (mandelbrot(c)) {
                pthread_mutex_lock(data->file_mutex);
                fprintf(data->file, "%lf,%lf\n", creal(c), cimag(c));
                pthread_mutex_unlock(data->file_mutex);
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s nthreads ntrials\n", argv[0]);
        return 1;
    }

    nthreads = strtol(argv[1], NULL, 10);
    npoints = strtol(argv[2], NULL, 10);

    FILE* file = fopen("mandelbrot.csv", "w");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    pthread_t threads[nthreads];

    clock_t start_time = clock();

    ThreadData thread_data[nthreads];
    pthread_mutex_t file_mutex;

    pthread_mutex_init(&file_mutex, NULL);

    for (int i = 0; i < nthreads; ++i) {
        thread_data[i] = (ThreadData){.id = i, .npoints = npoints, .nthreads = nthreads,
                                       .min_x = min_x, .max_x = max_x,
                                       .min_y = min_y, .max_y = max_y,
                                       .file = file, .file_mutex = &file_mutex};
        pthread_create(&threads[i], NULL, compute_mandelbrot, (void*)&thread_data[i]);
    }

    for (int i = 0; i < nthreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    clock_t end_time = clock();

    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Elapsed time: %.6f seconds\n", elapsed_time);

    pthread_mutex_destroy(&file_mutex);
    fclose(file);
    return 0;
}
