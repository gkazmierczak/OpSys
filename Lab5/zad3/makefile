CC = gcc -Wall

all: clean test clean

consumer.o:
	@$(CC) -c consumer.c

producer.o:
	@$(CC) -c producer.c

tester.o:
	@$(CC) -c tester.c

build: producer.o consumer.o tester.o
	@$(CC) producer.o -o producer
	@$(CC) consumer.o -o consumer
	@$(CC) tester.o -o tester

test: build many_to_one one_to_many many_to_many
	

many_to_one:
	@echo ------------------------------------
	@echo
	@rm -f fifo
	@mkfifo fifo
	@echo 5 Producers, 1 consumer test.
	@echo Testing for N=5.
	@./producer fifo 2 ./prod_files/file1 5 &
	@./producer fifo 4 ./prod_files/file2 5 &
	@./producer fifo 0 ./prod_files/file3 5 &
	@./producer fifo 6 ./prod_files/file4 5 &
	@./producer fifo 1 ./prod_files/file5 5 &
	@./consumer fifo out 7
	@./tester ./prod_files/file1 out 2 50
	@./tester ./prod_files/file2 out 4 50
	@./tester ./prod_files/file3 out 0 50
	@./tester ./prod_files/file4 out 6 50
	@./tester ./prod_files/file5 out 1 50
	@echo Testing for N=1024.
	@./producer fifo 2 ./prod_files/file1 1024 &
	@./producer fifo 4 ./prod_files/file2 1024 &
	@./producer fifo 0 ./prod_files/file3 1024 &
	@./producer fifo 6 ./prod_files/file4 1024 &
	@./producer fifo 1 ./prod_files/file5 1024 &
	@./consumer fifo out 1026
	@./tester ./prod_files/file1 out 2 1024
	@./tester ./prod_files/file2 out 4 1024
	@./tester ./prod_files/file3 out 0 1024
	@./tester ./prod_files/file4 out 6 1024
	@./tester ./prod_files/file5 out 1 1024
	@echo Testing for N=8192.
	@./producer fifo 2 ./prod_files/file1 8192 &
	@./producer fifo 4 ./prod_files/file2 8192 &
	@./producer fifo 0 ./prod_files/file3 8192 &
	@./producer fifo 6 ./prod_files/file4 8192 &
	@./producer fifo 1 ./prod_files/file5 8192 &
	@./consumer fifo out 8194
	@./tester ./prod_files/file1 out 2 8192
	@./tester ./prod_files/file2 out 4 8192
	@./tester ./prod_files/file3 out 0 8192
	@./tester ./prod_files/file4 out 6 8192
	@./tester ./prod_files/file5 out 1 8192
	@echo Finished tests for 5 Producers, 1 consumer.
	@echo
	@echo ------------------------------------

one_to_many:
	@echo ------------------------------------
	@echo
	@rm -f fifo
	@mkfifo fifo
	@echo 1 Producer, 5 consumers test.
	@echo Testing for N=5.
	@./consumer fifo out 7 &
	@./consumer fifo out 7 &
	@./consumer fifo out 7 &
	@./consumer fifo out 7 &
	@./consumer fifo out 7 &
	@./producer fifo 2 ./prod_files/file1 5 
	@./tester ./prod_files/file1 out 2 50
	@echo Testing for N=1024.
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 &
	@./producer fifo 2 ./prod_files/file1 1024 
	@./tester ./prod_files/file1 out 2 1024
	@echo Testing for N=8192.
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 &
	@./producer fifo 2 ./prod_files/file1 8192 
	@./tester ./prod_files/file1 out 2 8192
	@echo Finished tests for 1 Producer, 5 consumers.
	@echo
	@echo ------------------------------------

many_to_many:
	@echo ------------------------------------
	@echo
	@rm -f fifo
	@mkfifo fifo
	@echo 5 Producers, 5 consumers test.
	@echo Testing for N=5.
	@./producer fifo 2 ./prod_files/file1 5 &
	@./producer fifo 4 ./prod_files/file2 5 &
	@./producer fifo 0 ./prod_files/file3 5 &
	@./producer fifo 6 ./prod_files/file4 5 &
	@./producer fifo 1 ./prod_files/file5 5 &
	@./consumer fifo out 7 &
	@./consumer fifo out 7 &
	@./consumer fifo out 7 &
	@./consumer fifo out 7 &
	@./consumer fifo out 7 
	@./tester ./prod_files/file1 out 2 50
	@./tester ./prod_files/file2 out 4 50
	@./tester ./prod_files/file3 out 0 50
	@./tester ./prod_files/file4 out 6 50
	@./tester ./prod_files/file5 out 1 50
	@echo Testing for N=1024.
	@./producer fifo 2 ./prod_files/file1 1024 &
	@./producer fifo 4 ./prod_files/file2 1024 &
	@./producer fifo 0 ./prod_files/file3 1024 &
	@./producer fifo 6 ./prod_files/file4 1024 &
	@./producer fifo 1 ./prod_files/file5 1024 &
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 &
	@./consumer fifo out 1026 
	@./tester ./prod_files/file1 out 2 1024
	@./tester ./prod_files/file2 out 4 1024
	@./tester ./prod_files/file3 out 0 1024
	@./tester ./prod_files/file4 out 6 1024
	@./tester ./prod_files/file5 out 1 1024
	@echo Testing for N=8192.
	@./producer fifo 2 ./prod_files/file1 8192 &
	@./producer fifo 4 ./prod_files/file2 8192 &
	@./producer fifo 0 ./prod_files/file3 8192 &
	@./producer fifo 6 ./prod_files/file4 8192 &
	@./producer fifo 1 ./prod_files/file5 8192 &
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 &
	@./consumer fifo out 8194 
	@./tester ./prod_files/file1 out 2 8192
	@./tester ./prod_files/file2 out 4 8192
	@./tester ./prod_files/file3 out 0 8192
	@./tester ./prod_files/file4 out 6 8192
	@./tester ./prod_files/file5 out 1 8192
	@echo Finished tests for 5 Producers, 5 consumers.
	@echo
	@echo ------------------------------------

clean:
	@rm -f *.o