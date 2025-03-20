#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define N 100000000          // Número total de indivíduos
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

/**
 * Inicializa a população de pessoas com posições aleatórias
 * em um quadrado unitário e status (infectado ou suscetível)
 * também aleatório, com 10% dos indivíduos começando infectados.
 *
 * @param p o vetor de pessoas a ser inicializado
 * @param n o tamanho do vetor de pessoas
 */
void inicializar_populacao(Pessoa *p, int n) {
    for (int i = 0; i < n; i++) {
        p[i].x = (double) rand() / RAND_MAX;
        p[i].y = (double) rand() / RAND_MAX;
        p[i].status = (rand() % 100 < 10) ? INFECTADO : SUSCETIVEL; 
    }
}

/**
 * Atualiza o estado de cada indivíduo na população.
 *
 * Itera sobre todos os indivíduos e, para cada indivíduo infectado, 
 * verifica a proximidade com outros indivíduos suscetíveis. Se a 
 * distância entre eles for menor que DIST_MAX e uma condição aleatória 
 * baseada na taxa de infecção BETA for atendida, o indivíduo 
 * suscetível se torna infectado. Além disso, um indivíduo infectado 
 * pode se recuperar com uma probabilidade determinada pela taxa de 
 * recuperação GAMMA.
 *
 * @param p Ponteiro para o array de indivíduos da população.
 * @param n Número de indivíduos na população.
 */
void atualizar_estado(Pessoa *p, int n) {
    // Usar OpenMP para paralelizar o loop principal
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n; i++) {
        // Verifica se o indivíduo está infectado
        if (p[i].status == INFECTADO) {
            // Itera sobre todos os indivíduos para verificar suscetíveis
            for (int j = 0; j < n; j++) {
                if (p[j].status == SUSCETIVEL) {
                    // Calcula a distância ao quadrado entre os indivíduos
                    double dx = p[i].x - p[j].x;
                    double dy = p[i].y - p[j].y;
                    double dist = dx * dx + dy * dy;  // Evita usar sqrt para melhor performance
                    // Verifica se a distância é menor que a máxima permitida e se a condição de infecção é atendida
                    if (dist < DIST_MAX * DIST_MAX && ((double) rand() / RAND_MAX) < BETA) {
                        // Atualiza o estado para infectado de forma atômica
                        #pragma omp atomic write
                        p[j].status = INFECTADO;
                    }
                }
            }
            // Verifica se o indivíduo infectado se recupera
            if (((double) rand() / RAND_MAX) < GAMMA) {
                // Atualiza o estado para recuperado de forma atômica
                #pragma omp atomic write
                p[i].status = RECUPERA;
            }
        }
    }
}

/**
 * Exibe estatísticas sobre o estado da população em uma dada iteração,
 * como o número de suscetíveis, infectados e recuperados.
 *
 * @param pop_global Ponteiro para o array de indivíduos da população.
 * @param n Número de indivíduos na população.
 */
void exibir_estatisticas(Pessoa *pop_global, int n) {
    int suscetiveis = 0, infectados = 0, recuperados = 0;
    
    #pragma omp parallel for reduction(+: suscetiveis, infectados, recuperados)
    for (int i = 0; i < n; i++) {
        switch (pop_global[i].status) {
            case SUSCETIVEL:
                suscetiveis++;
                break;
            case INFECTADO:
                infectados++;
                break;
            case RECUPERA:
                recuperados++;
                break;
        }
    }

    printf("Suscetíveis: %d, Infectados: %d, Recuperados: %d\n", 
            suscetiveis, infectados, recuperados);
}

/**
 * Função principal do programa.
 *
 * A função inicializa a população global em um rank 0 e a distribui para
 * todos os ranks. Em seguida, cada rank executa o loop principal com o
 * método atualizar_estado e, em seguida, o rank 0 recebe todos os dados
 * e exibe estatísticas sobre o estado da população em uma dada iteração.
 *
 * @param argc O número de argumentos passados na linha de comando.
 * @param argv Um vetor de strings com os argumentos passados na linha de comando.
 *
 * @return 0 se a execução for bem-sucedida; 1 caso contrário.
 */
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
        // Alocar memória para a população global
        pop_global = malloc(N * sizeof(Pessoa));
        if (pop_global == NULL) {
            fprintf(stderr, "Erro ao alocar memória para pop_global\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        // Inicializar população global
        inicializar_populacao(pop_global, N);
    }

    // Distribuir os dados corretamente
    MPI_Scatter(pop_global, n_local, MPI_PESSOA, 
                p_local, n_local, MPI_PESSOA, 
                0, MPI_COMM_WORLD);

    for (int t = 0; t < STEPS; t++) {
        // Atualizar estado dos indivíduos em cada rank
        atualizar_estado(p_local, n_local);
        // Aguardar que todos os ranks tenham concluído a atualização
        MPI_Barrier(MPI_COMM_WORLD);
        
        // O rank 0 recebe os dados de volta
        MPI_Gather(p_local, n_local, MPI_PESSOA, 
                   pop_global, n_local, MPI_PESSOA, 
                   0, MPI_COMM_WORLD);

        if (rank == 0) {
            // Exibir estatísticas sobre o estado da população
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

