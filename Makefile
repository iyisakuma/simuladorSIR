CC = gcc
CFLAGS = -Wall -O2

all: sir_sim

sir_sim: main.o sir.o
	$(CC) $(CFLAGS) main.o sir.o -o sir_sim

main.o: main.c sir.h
	$(CC) $(CFLAGS) -c main.c

sir.o: sir.c sir.h
	$(CC) $(CFLAGS) -c sir.c

clean:
	rm -f *.o sir_sim

