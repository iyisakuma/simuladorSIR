#include "sir.h"
#include <stdio.h>

// Inicializa a população em posições aleatórias
void inicializar_populacao(Pessoa *populacao) {
    for (int i = 0; i < N; i++) {
        populacao[i].x = (double)(rand() % 100);
        populacao[i].y = (double)(rand() % 100);
        populacao[i].status = (rand() % 100 < 5) ? INFECTADO : SUSCEPTIVEL; // 5% começam infectados
    }
}

// Atualiza os estados de cada indivíduo na simulação
void atualizar_estado(Pessoa *populacao) {
    for (int i = 0; i < N; i++) {
        if (populacao[i].status == SUSCEPTIVEL) {
            for (int j = 0; j < N; j++) {
                if (populacao[j].status == INFECTADO) {
                    double dist = sqrt(pow(populacao[i].x - populacao[j].x, 2) + 
                                       pow(populacao[i].y - populacao[j].y, 2));
                    if (dist < DISTANCIA_INF && ((double) rand() / RAND_MAX) < BETA) {
                        populacao[i].status = INFECTADO;
                        break;
                    }
                }
            }
        } else if (populacao[i].status == INFECTADO) {
            if (((double)rand() / RAND_MAX) < GAMMA) {
                populacao[i].status = RECUPERADO;
            }
        }
    }
}

// Salva os dados da simulação em um arquivo para análise
void salvar_dados(Pessoa *populacao, int tempo) {
    FILE *arquivo = fopen("dados_sir.txt", "a");
    int S = 0, I = 0, R = 0;
    
    for (int i = 0; i < N; i++) {
        if (populacao[i].status == SUSCEPTIVEL) S++;
        else if (populacao[i].status == INFECTADO) I++;
        else R++;
    }

    fprintf(arquivo, "%d %d %d %d\n", tempo, S, I, R);
    fclose(arquivo);
}

