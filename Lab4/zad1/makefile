CC = gcc -Wall 

all: build clean

main.o:
	$(CC) -c main.c

exec.o:
	$(CC) -c exec.c
	
child.o:
	$(CC) -c child.c

build: main.o exec.o child.o
	$(CC) main.o -o zad1
	$(CC) exec.o -o exec
	$(CC) child.o -o child

clean:
	rm -f *.o