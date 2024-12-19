#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>

#define M 5
#define N 8

/* Mutex for shared accumulation */
pthread_mutex_t mutex;

/* Structure pour passer les arguments aux threads */
typedef struct {
    size_t row;               // Ligne de la matrice
    size_t n;                 // Nombre de colonnes
    double (*A)[N];           // Pointeur vers la matrice
    double *shared_frob;      // Pointeur vers la somme partagée
} ThreadData;

/* Fonction exécutée par chaque thread */
void* compute_row(void* args) {
    ThreadData* data = (ThreadData*)args;
    double local_sum = 0.;

    // Calcul de la somme des carrés pour une ligne donnée
    for (size_t j = 0; j < data->n; ++j) {
        local_sum += data->A[data->row][j] * data->A[data->row][j];
    }

    // Accumulation dans la somme partagée
    pthread_mutex_lock(&mutex);
    *(data->shared_frob) += local_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

/* Référence : Frobenius norm (version séquentielle) */
double frobenius_ref(size_t m, size_t n, double A[m][n]) {
    double frob = 0.;
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            frob += A[i][j] * A[i][j];
        }
    }
    return sqrt(frob);
}

/* Frobenius norm avec pthreads */
double frobenius(size_t m, size_t n, double A[m][n]) {
    pthread_t threads[m];
    ThreadData thread_data[m];
    double frob = 0.;

    // Initialiser le mutex
    pthread_mutex_init(&mutex, NULL);

    // Créer un thread pour chaque ligne
    for (size_t i = 0; i < m; ++i) {
        thread_data[i].row = i;
        thread_data[i].n = n;
        thread_data[i].A = A;
        thread_data[i].shared_frob = &frob;

        pthread_create(&threads[i], NULL, compute_row, &thread_data[i]);
    }

    // Attendre la fin de tous les threads
    for (size_t i = 0; i < m; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Détruire le mutex
    pthread_mutex_destroy(&mutex);

    return sqrt(frob);
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

inline bool isClose(double a, double b, double threshold) {
    return fabs(a - b) < threshold;
}

/* MAIN */

int main(void) {
    size_t n = N;
    size_t m = M;

    double A[m][n];
    initMatrix(m, n, A);

    printf("A=\n");
    printMatrix(m, n, A);

    double ref = frobenius_ref(m, n, A);
    double res = frobenius(m, n, A);

    printf("\nref=%lf res=%lf\n", ref, res);
    if (isClose(ref, res, 0.0001)) {
        printf("OK\n");
    } else {
        printf("ERROR: difference between ref and res is above threshold\n");
    }

    return 0;
}
