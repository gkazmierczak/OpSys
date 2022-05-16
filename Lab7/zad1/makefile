CC = gcc -Wall 
LIB_NAME = common

all: build clean

static:
	$(CC) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

build: static
	$(CC) -c main.c
	$(CC) -c cook.c
	$(CC) -c courier.c
	$(CC) main.o $(LIB_NAME).a -o zad1
	$(CC) cook.o $(LIB_NAME).a -o cook
	$(CC) courier.o $(LIB_NAME).a -o courier

rebuild:
	rm -f zad1 cook courier common.a
	make build
	make clean

clean:
	rm -f *.o