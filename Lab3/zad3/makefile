CC = gcc -Wall 

all: build clean

main.o:
	$(CC) -c main.c

build: main.o
	$(CC) main.o -o zad3

clean:
	rm -f *.o