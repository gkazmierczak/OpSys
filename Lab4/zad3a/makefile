CC = gcc -Wall

all: build clean


sender.o:
	$(CC) -c sender.c

catcher.o:
	$(CC) -c catcher.c

build: sender.o catcher.o
	$(CC) sender.o -o sender
	$(CC) catcher.o -o catcher

clean:
	rm -f *.o