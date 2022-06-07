CC = gcc -Wall 

all: build clean

main.o:
	$(CC) -c server.c
	$(CC) -c client.c


build: main.o
	$(CC) server.o -o server -lpthread
	$(CC) client.o -o client -lpthread

clean:
	rm -f *.o