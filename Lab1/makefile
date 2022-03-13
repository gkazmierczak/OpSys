CC = gcc -Wall 
LIB_NAME = library

all: static shared clean

static:
	$(CC) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

shared:
	$(CC) -c -fPIC $(LIB_NAME).c
	$(CC) -shared -o $(LIB_NAME).so $(LIB_NAME).o

clean:
	rm -f *.o