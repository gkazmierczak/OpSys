CC = gcc -Wall 

all: build clean

main.o:
	$(CC) -c main.c

build: main.o
	$(CC) main.o -o zad1

clean:
	rm -f *.o