#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <pthread.h>
#include <assert.h>

#define N 10

double dotprod_ref(size_t n, double a[n], double b[n]); // Déclaration uniquement

typedef struct {
    size_t index;
    double *a;
    double *b;
    double *sum;
    pthread_mutex_t *mutex;
} thread_data_t;

void* thread_dotprod(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    double local_sum = data->a[data->index] * data->b[data->index];
    
    pthread_mutex_lock(data->mutex);
    *(data->sum) += local_sum;
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

double dotprod_pairs(size_t n, double a[n], double b[n]) {
    pthread_t threads[n];
    thread_data_t thread_data[n];
    pthread_mutex_t mutex;
    double sum = 0.0;

    pthread_mutex_init(&mutex, NULL);

    for (size_t i = 0; i < n; ++i) {
        thread_data[i] = (thread_data_t){
            .index = i,
            .a = a,
            .b = b,
            .sum = &sum,
            .mutex = &mutex
        };
        pthread_create(&threads[i], NULL, thread_dotprod, &thread_data[i]);
    }

    for (size_t i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return sum;
}

void initArray(size_t n, double a[n]) {
    static double elem = 0.;
    for(size_t i = 0; i < n; ++i) {
        a[i] = elem;
        elem += 1.;
    }
}

void printArray(size_t n, double a[n]) {
    for(size_t i = 0; i < n; ++i) {
        printf("%lf ", a[i]);
    }
    printf("\n");
}

inline bool isClose(double a, double b, double threshold) {
    return fabs(a - b) < threshold;
}

int main(void) {
    size_t n = N;

    double a[n]; double b[n];
    initArray(n, a);
    initArray(n, b);

    printf("a=\n");
    printArray(n, a);
    printf("b=\n");
    printArray(n, b);

    double ref = dotprod_ref(n, a, b);
    double res = dotprod_pairs(n, a, b);
    
    printf("\nref=%lf res=%lf\n", ref, res);
    if (isClose(ref, res, 0.0001)) {
        printf("OK\n");
    } else {
        printf("ERROR: difference between ref and res is above threshold\n");
    }

    return 0;
}
