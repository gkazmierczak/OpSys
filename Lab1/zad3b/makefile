CC = gcc -Wall 
LIB_NAME = library
MAIN = main
NAME = zad3
Olevel=O0

all: static shared clean

static:
	$(CC) -c $(LIB_NAME).c -$(Olevel)
	ar rcs $(LIB_NAME).a $(LIB_NAME).o

shared:
	$(CC) -c -fPIC $(LIB_NAME).c -$(Olevel)
	$(CC) -shared -o $(LIB_NAME).so $(LIB_NAME).o


zad3_static: static
	$(CC) -c $(MAIN).c -$(Olevel)
	$(CC) $(MAIN).o $(LIB_NAME).a -o $(NAME)

zad3_shared: shared
	$(CC) -c $(MAIN).c -$(Olevel)
	$(CC) $(MAIN).o -L. $(LIB_NAME).so -Wl,-rpath=. -o $(NAME)

zad3_dynamic: shared
	gcc $(MAIN)_dynamic.c -o $(NAME) -ldl -$(Olevel)
	
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
	./$(NAME) -c 10 -h -t -w small -e "single small file" -t -w medium -e "single medium file" -t -w large -e "single large file" -t -w m1 m2 m3 -e "3 medium sized files" -t -w m1 l1 s1 m2 s2 l2 l3 s3 m3 m1 -e "10 files" -t -w m1 l1 l2 m2 m3 -e "5 files"
	./$(NAME) -c 10 -w medium -t -s -e "store tempfile after single file count" -w s1 s2 s3 -t -s -e "store tempfile after 3 counts"  -t -r 0 -e "free block from single count" -t -r 1 -e "free block from 3 counts"
	cp large results.tmp
	./$(NAME) -c 10 -t -s -e "store 8MB tempfile" -t -r 0 -e "free block from 8mb tempfile"
	./$(NAME) -c 10 -t -i 5 2 1024 -d 5 2 -i 5 2 1024 -d 5 2 -i 5 2 1024 -d 5 2 -e "store and free 5x 1KB blocks 3 times"
	./$(NAME) -c 100 -t -a 20 50 12 2048 -e "add and delete 50x 2KB blocks 20 times"
	./$(NAME) -c 100 -t -a 2000 50 12 4096 -e "add and delete 50x 4KB blocks 2000 times"

tests_100M:
	make file100M
	mv bigfile results.tmp
	./$(NAME) -c 10 -h -t -s -e "store 100MB tempfile" -t -r 0 -e "free block from 100mb tempfile"

all_test:
	make random
	touch results3b.txt
	make zad3_static
	make run_tests
	echo "Test results - static, -$(Olevel):" >> results3b.txt
	cat results3.txt >> results3b.txt
	rm -f results3.txt
	rm -f $(NAME)
	make zad3_shared
	make run_tests
	echo "Test results - shared, -$(Olevel):" >> results3b.txt
	cat results3.txt >> results3b.txt
	rm -f results3.txt
	rm -f $(NAME)
	make zad3_dynamic
	make run_tests
	echo "Test results - dynamic, -$(Olevel):" >> results3b.txt
	cat results3.txt >> results3b.txt
	rm -f results3.txt
	rm -f $(NAME)

static_test: zad3_static
	make run_tests
	echo "Test results - static, -$(Olevel):" >> results3b.txt
	cat results3.txt >> results3b.txt
	rm -f results3.txt
	rm -f $(NAME)

shared_test: zad3_shared
	make run_tests
	echo "Test results - shared, -$(Olevel):" >> results3b.txt
	cat results3.txt >> results3b.txt
	rm -f results3.txt
	rm -f $(NAME)

dynamic_test: zad3_dynamic
	make run_tests
	echo "Test results - dynamic, -$(Olevel):" >> results3b.txt
	cat results3.txt >> results3b.txt
	rm -f results3.txt
	rm -f $(NAME)

all_Olevels_test:
	make random
	make static_test Olevel=O0
	make static_test Olevel=O1
	make static_test Olevel=O2
	make static_test Olevel=O3
	make static_test Olevel=Os
	make dynamic_test Olevel=O0
	make dynamic_test Olevel=O1
	make dynamic_test Olevel=O2
	make dynamic_test Olevel=O3
	make dynamic_test Olevel=Os
	make shared_test Olevel=O0
	make shared_test Olevel=O1
	make shared_test Olevel=O2
	make shared_test Olevel=O3
	make shared_test Olevel=Os
