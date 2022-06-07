CC = gcc -Wall 

all: build clean

main.o:
	$(CC) -c main.c 

build: main.o
	$(CC) main.o -o main -lpthread

clean:
	rm -f *.o