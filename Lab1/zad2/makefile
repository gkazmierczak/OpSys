CC = gcc -Wall 
LIB_NAME = library
MAIN = main
NAME = zad2

all: static shared clean

static:
	$(CC) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

shared:
	$(CC) -c -fPIC $(LIB_NAME).c
	$(CC) -shared -o $(LIB_NAME).so $(LIB_NAME).o

zad2_static: static
	$(CC) -c $(MAIN).c
	$(CC) $(MAIN).o $(LIB_NAME).a -o $(NAME)

zad2_shared: shared
	$(CC) -c $(MAIN).c
	$(CC) $(MAIN).o -L. $(LIB_NAME).so -Wl,-rpath=. -o $(NAME)
	
clean:
	rm -f *.o

random:
	head -c 32 /dev/random | base64 | head -c 32 > small
	head -c 32 /dev/random | base64 | head -c 32 > s1
	head -c 32 /dev/random | base64 | head -c 32 > s2
	head -c 32 /dev/random | base64 | head -c 32 > s3
	head -c 512K /dev/random | base64 | head -c 512K > medium
	head -c 512K /dev/random | base64 | head -c 512K > m1
	head -c 512K /dev/random | base64 | head -c 512K > m2
	head -c 512K /dev/random | base64 | head -c 512K > m3
	head -c 8M /dev/random | base64 | head -c 8M > large
	head -c 8M /dev/random | base64 | head -c 8M > l1
	head -c 8M /dev/random | base64 | head -c 8M > l2
	head -c 8M /dev/random | base64 | head -c 8M > l3

file100M:
	head -c 100M /dev/random | base64 | head -c 100M > bigfile

run_tests:
	make random
	./$(NAME) -c 10 -h -t -w small -e "single small file" -t -w medium -e "single medium file" -t -w large -e "single large file" -t -w m1 m2 m3 -e "3 medium sized files" -t -w m1 l1 s1 m2 s2 l2 l3 s3 m3 m1 -e "10 files" -t -w m1 l1 l2 m2 m3 -e "5 files"
	./$(NAME) -c 10 -w medium -t -s -e "store tempfile after single file count" -w s1 s2 s3 -t -s -e "store tempfile after 3 counts"  -t -r 0 -e "free block from single count" -t -r 1 -e "free block from 3 counts"
	mv large results.tmp
	./$(NAME) -c 10 -t -s -e "store 8MB tempfile" -t -r 0 -e "free block from 8mb tempfile"
	./$(NAME) -c 10 -t -i 5 2 1024 -d 5 2 -i 5 2 1024 -d 5 2 -i 5 2 1024 -d 5 2 -e "store and free 5x 1KB blocks 3 times"
	./$(NAME) -c 100 -t -a 20 50 12 2048 -e "add and delete 50x 2KB blocks 20 times"
	./$(NAME) -c 100 -t -a 2000 50 12 4096 -e "add and delete 50x 4KB blocks 2000 times"

tests_100M:
	make file100M
	mv bigfile results.tmp
	./$(NAME) -c 10 -h -t -s -e "store 100MB tempfile" -t -r 0 -e "free block from 100mb tempfile"

test_static: zad2_static
	make run_tests

test_shared: zad2_shared
	make run_tests