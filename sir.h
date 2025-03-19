
#define SIR_H

#include <stdlib.h>
#include <math.h>

// Definição de estados do indivíduo
#define SUSCEPTIVEL 0
#define INFECTADO 1
#define RECUPERADO 2


// Parâmetros do modelo
#define N 1000          // População total
#define BETA 0.1        // Taxa de infecção
#define GAMMA 0.05      // Taxa de recuperação
#define TEMPO_MAX 100   // Tempo de simulação
#define DISTANCIA_INF 2.0  // Distância máxima para infecção

// Estrutura para representar um indivíduo
typedef struct {
    int status;  // 0 = Suscetível, 1 = Infectado, 2 = Recuperado
    double x, y; // Posição do indivíduo no espaço 2D
} Pessoa;

// Funções para manipulação da simulação
void inicializar_populacao(Pessoa *populacao);
void atualizar_estado(Pessoa *populacao);
void salvar_dados(Pessoa *populacao, int tempo);


