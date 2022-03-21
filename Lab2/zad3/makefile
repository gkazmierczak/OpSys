CC = gcc -Wall 

all: build_dir build_nftw clean

main_dir.o: main_dir.c
	$(CC) -c main_dir.c

build_dir: main_dir.o
	$(CC) main_dir.o -o zad3_dir

main_nftw.o:
	$(CC) -c main_nftw.c

build_nftw: main_nftw.o
	$(CC) main_nftw.o -o zad3_nftw

clean:
	rm -f *.o