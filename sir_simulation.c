#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define N 10000           // Número total de indivíduos
#define STEPS 100        // Número de iterações
#define DIST_MAX 0.02    // Distância máxima para contágio
#define BETA 0.3         // Taxa de infecção
#define GAMMA 0.1        // Taxa de recuperação
#define INFECTADO 1
#define SUSCETIVEL 0
#define RECUPERA 2

typedef struct {
    double x, y;
    int status; 
} Pessoa;

void inicializar_populacao(Pessoa *p, int n) {
    #pragma omp parallel for 
    for (int i = 0; i < n; i++) {
        p[i].x = (double) rand() / RAND_MAX;
        p[i].y = (double) rand() / RAND_MAX;
        p[i].status = (rand() % 100 < 10) ? INFECTADO : SUSCETIVEL; // 10% começam infectados
    }
}

void atualizar_estado(Pessoa *p, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (p[i].status == INFECTADO) { 
            for (int j = 0; j < n; j++) {
                if (p[j].status == SUSCETIVEL) {
                    double dist = sqrt(pow(p[i].x - p[j].x, 2) + pow(p[i].y - p[j].y, 2));
                    if (dist < DIST_MAX && ((double) rand() / RAND_MAX) < BETA) {
                        p[j].status = INFECTADO;
                    }
                }
            }
            if (((double) rand() / RAND_MAX) < GAMMA) {
                p[i].status = RECUPERA;
            }
        }
    }
}

void exibir_estatisticas(Pessoa *pop_global, int n) {
    int suscetiveis = 0, infectados = 0, recuperados = 0;
    
    for (int i = 0; i < n; i++) {
        if (pop_global[i].status == SUSCETIVEL)
            suscetiveis++;
        else if (pop_global[i].status == INFECTADO)
            infectados++;
        else if (pop_global[i].status == RECUPERA)
            recuperados++;
    }

    printf("Suscetíveis: %d, Infectados: %d, Recuperados: %d\n", 
            suscetiveis, infectados, recuperados);
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);         
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Criar tipo MPI para Pessoa
    MPI_Datatype MPI_PESSOA;
    int block_lengths[2] = {2, 1};  // Dois doubles (x, y) e um int (status)
    MPI_Aint offsets[2];
    offsets[0] = offsetof(Pessoa, x);
    offsets[1] = offsetof(Pessoa, status);
    MPI_Datatype types[2] = {MPI_DOUBLE, MPI_INT};
    
    MPI_Type_create_struct(2, block_lengths, offsets, types, &MPI_PESSOA);
    MPI_Type_commit(&MPI_PESSOA);

    int n_local = N / size;
    Pessoa *p_local = malloc(n_local * sizeof(Pessoa));

    Pessoa *pop_global = NULL;
    if (rank == 0) {
        pop_global = malloc(N * sizeof(Pessoa));
        if (pop_global == NULL) {
            fprintf(stderr, "Erro ao alocar memória para pop_global\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        inicializar_populacao(pop_global, N);
    }

    // Distribuir os dados corretamente
    MPI_Scatter(pop_global, n_local, MPI_PESSOA, 
                p_local, n_local, MPI_PESSOA, 
                0, MPI_COMM_WORLD);

    for (int t = 0; t < STEPS; t++) {
        atualizar_estado(p_local, n_local);
        MPI_Barrier(MPI_COMM_WORLD);
        
        // O rank 0 recebe os dados de volta
        MPI_Gather(p_local, n_local, MPI_PESSOA, 
                   pop_global, n_local, MPI_PESSOA, 
                   0, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("Iteração %d:\n", t);
            exibir_estatisticas(pop_global, N);
        }
    }

    // Liberar memória e destruir tipo MPI
    free(p_local);
    if (rank == 0) free(pop_global);
    MPI_Type_free(&MPI_PESSOA);
    MPI_Finalize();
    return 0;
}

