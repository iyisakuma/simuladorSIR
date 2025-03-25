# 1. Visão Geral

Este programa simula a propagação de uma doença infecciosa entre indivíduos distribuídos aleatoriamente em um espaço bidimensional. O modelo adotado é o SIR, que classifica as pessoas como:

    Suscetíveis (S): podem ser infectadas;

    Infectadas (I): estão com a doença e podem contagiar;

    Recuperadas (R): se curaram e não são mais suscetíveis.

A simulação roda por várias iterações (STEPS) e a interação entre os indivíduos ocorre caso estejam a uma distância inferior a um determinado limiar (DIST_MAX).

O uso de MPI (Message Passing Interface) permite distribuir a simulação entre vários processos, com cada processo sendo responsável por uma parte da população. Já o OpenMP explora o paralelismo intra-processo (em múltiplos threads) para acelerar os cálculos locais.

# 2. Pre-requisitos

* gcc
* mpicc


# 3. Compilar em ambiente linux

    mpicc -o sir_simulation sir_simulation.c -fopenmp -lm

# 4. Rodar o executavel

    mpirun -np 4 ./sir_simulation
