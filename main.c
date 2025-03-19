#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define N 1000           // Número total de indivíduos
#define STEPS 100        // Número de iterações
#define DIST_MAX 0.02    // Distância máxima para contágio
#define BETA 0.3         // Taxa de infecção
#define GAMMA 0.1        // Taxa de recuperação

typedef struct {
    double x, y;
    int status; // 0 = S, 1 = I, 2 = R
} Pessoa;

void inicializar_populacao(Pessoa *pop, int n) {
    for (int i = 0; i < n; i++) {
        pop[i].x = (double) rand() / RAND_MAX;
        pop[i].y = (double) rand() / RAND_MAX;
        pop[i].status = (rand() % 100 < 5) ? 1 : 0; // 5% começam infectados
    }
}

void atualizar_estado(Pessoa *pop, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (pop[i].status == 1) { // Infectado
            for (int j = 0; j < n; j++) {
                if (pop[j].status == 0) { // Suscetível
                    double dist = sqrt(pow(pop[i].x - pop[j].x, 2) + pow(pop[i].y - pop[j].y, 2));
                    if (dist < DIST_MAX && ((double) rand() / RAND_MAX) < BETA) {
                        pop[j].status = 1; // Infecta
                    }
                }
            }
            if (((double) rand() / RAND_MAX) < GAMMA) {
                pop[i].status = 2; // Recupera
            }
        }
    }
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);         
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n_local = N / size;
    Pessoa *pop_local = malloc(n_local * sizeof(Pessoa));

    if (rank == 0) {
        Pessoa *pop_global = malloc(N * sizeof(Pessoa));
        inicializar_populacao(pop_global, N);
        MPI_Scatter(pop_global, n_local * sizeof(Pessoa), MPI_BYTE, pop_local, n_local * sizeof(Pessoa), MPI_BYTE, 0, MPI_COMM_WORLD);
        free(pop_global);
    } else {
        MPI_Scatter(NULL, 0, MPI_BYTE, pop_local, n_local * sizeof(Pessoa), MPI_BYTE, 0, MPI_COMM_WORLD);
    }

    for (int t = 0; t < STEPS; t++) {
        atualizar_estado(pop_local, n_local);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gather(pop_local, n_local * sizeof(Pessoa), MPI_BYTE, NULL, 0, MPI_BYTE, 0, MPI_COMM_WORLD);
    }

    free(pop_local);
    MPI_Finalize();
    return 0;
}

