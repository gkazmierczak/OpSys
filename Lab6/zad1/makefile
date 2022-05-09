CC = gcc -Wall 
LIB_NAME = chat

all: build clean

static:
	$(CC) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

build: static
	$(CC) -c server.c
	$(CC) -c client.c
	$(CC) server.o $(LIB_NAME).a -o server
	$(CC) client.o $(LIB_NAME).a -o client

rebuild:
	rm -f client server chat.a
	make build
	make clean

clean:
	rm -f *.o