CC = gcc -Wall 
LIB_NAME = library
MAIN = main
EXPORT = zad2

all: static shared clean

static:
	$(CC) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

shared:
	$(CC) -c -fPIC $(LIB_NAME).c
	$(CC) -shared -o $(LIB_NAME).so $(LIB_NAME).o

zad2_static:
	make static
	$(CC) -c $(MAIN).c
	$(CC) $(MAIN).o $(LIB_NAME).a -o $(EXPORT)

zad2_shared:
	make shared
	$(CC) -c $(MAIN).c
	$(CC) $(MAIN).o -L. $(LIB_NAME).so -Wl,-rpath=. -o $(EXPORT)
	
clean:
	rm -f *.o

tests:
	$(EXPORT) -c 10 -w 

random:
	head -c 32 /dev/random | base64 | head -c 32 > small
	head -c 512K /dev/random | base64 | head -c 512K > medium
	head -c 8M /dev/random | base64 | head -c 8M > large
