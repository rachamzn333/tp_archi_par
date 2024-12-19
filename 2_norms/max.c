#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>

#define M 5
#define N 8

/* Structure pour les données des threads */
typedef struct {
    size_t row;             // Ligne à traiter
    size_t n;               // Nombre de colonnes
    double (*A)[N];         // Matrice
    double *globalMax;      // Variable partagée pour le maximum global
    pthread_mutex_t *mutex; // Mutex pour protéger la variable partagée
} thread_data_t;

/* Fonction exécutée par chaque thread */
void* thread_max(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    double localMax = fabs(data->A[data->row][0]);

    // Trouver le maximum local de la ligne
    for (size_t j = 1; j < data->n; ++j) {
        if (fabs(data->A[data->row][j]) > localMax) {
            localMax = fabs(data->A[data->row][j]);
        }
    }

    // Mettre à jour le maximum global avec le mutex
    pthread_mutex_lock(data->mutex);
    if (localMax > *(data->globalMax)) {
        *(data->globalMax) = localMax;
    }
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

/* Fonction séquentielle de référence */
double max_ref(size_t m, size_t n, double A[m][n]) {
    double maxElem = fabs(A[0][0]);
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (fabs(A[i][j]) > maxElem) {
                maxElem = fabs(A[i][j]);
            }
        }
    }
    return maxElem;
}

/* Fonction parallèle */
double max(size_t m, size_t n, double A[m][n]) {
    pthread_t threads[m];
    thread_data_t thread_data[m];
    pthread_mutex_t mutex;
    double globalMax = fabs(A[0][0]);

    pthread_mutex_init(&mutex, NULL);

    // Création des threads
    for (size_t i = 0; i < m; ++i) {
        thread_data[i] = (thread_data_t){
            .row = i,
            .n = n,
            .A = A,
            .globalMax = &globalMax,
            .mutex = &mutex
        };
        pthread_create(&threads[i], NULL, thread_max, &thread_data[i]);
    }

    // Attente de la fin des threads
    for (size_t i = 0; i < m; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    return globalMax;
}

/* UTILS */
void initMatrix(size_t m, size_t n, double A[m][n]) {
    static double elem = 0.;
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            A[i][j] = elem;
            elem += 1.;
        }
    }
}

void printMatrix(size_t m, size_t n, double A[m][n]) {
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            printf("%lf ", A[i][j]);
        }
        printf("\n");
    }
}

/* MAIN */
int main(void) {
    size_t n = N;
    size_t m = M;

    double A[m][n];
    initMatrix(m, n, A);

    printf("A=\n");
    printMatrix(m, n, A);

    double ref = max_ref(m, n, A);
    double res = max(m, n, A);

    printf("\nref=%lf res=%lf\n", ref, res);
    if (ref == res) {
        printf("OK\n");
    } else {
        printf("ERROR: difference between ref and res is above threshold\n");
    }

    return 0;
}
