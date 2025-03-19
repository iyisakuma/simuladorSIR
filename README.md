# Compilar em ambiente linux
 mpicc -o sir_simulation sir_simulation.c -fopenmp -lm
# Rodar o executavek
mpirun -np 4 ./sir_simulation
