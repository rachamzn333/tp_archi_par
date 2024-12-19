#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <pthread.h>
#include <assert.h>

#define N 9
#define K 3

double dotprod_ref(size_t n, double a[n], double b[n]);

/* Structure pour les données des threads */
typedef struct {
    size_t start;        // Index de début du bloc
    size_t end;          // Index de fin du bloc
    double *a;           // Tableau a
    double *b;           // Tableau b
    double *sum;         // Variable partagée pour accumuler le résultat
    pthread_mutex_t *mutex; // Mutex pour protéger l'accès à sum
} thread_data_t;

/* Fonction exécutée par chaque thread */
void* thread_dotprod_blocks(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    double local_sum = 0.0;

    for (size_t i = data->start; i < data->end; ++i) {
        local_sum += data->a[i] * data->b[i];
    }

    // Section critique pour accumuler local_sum dans sum
    pthread_mutex_lock(data->mutex);
    *(data->sum) += local_sum;
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

/* DOTPROD - Parallélisation par blocs */
double dotprod_blocks(size_t n, size_t k, double a[n], double b[n]) {
    size_t num_threads = n / k; // Nombre de threads
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    pthread_mutex_t mutex;
    double sum = 0.0;

    pthread_mutex_init(&mutex, NULL);

    // Création des threads
    for (size_t t = 0; t < num_threads; ++t) {
        thread_data[t] = (thread_data_t){
            .start = t * k,
            .end = (t + 1) * k,
            .a = a,
            .b = b,
            .sum = &sum,
            .mutex = &mutex
        };
        pthread_create(&threads[t], NULL, thread_dotprod_blocks, &thread_data[t]);
    }

    // Attente de la terminaison des threads
    for (size_t t = 0; t < num_threads; ++t) {
        pthread_join(threads[t], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return sum;
}

/* UTILS */

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

/* MAIN */

int main(void) {
    size_t n = N;
    size_t k = K;

    double a[n]; double b[n];
    initArray(n, a);
    initArray(n, b);

    printf("a=\n");
    printArray(n, a);
    printf("b=\n");
    printArray(n, b);

    double ref = dotprod_ref(n, a, b);
    double res = dotprod_blocks(n, k , a, b);
    
    printf("\nref=%lf res=%lf\n", ref, res);
    if(isClose(ref, res, 0.0001)) {
        printf("OK\n");
    } else {
        printf("ERROR: difference between ref and res is above threshold\n");
    }

    return 0;
}
