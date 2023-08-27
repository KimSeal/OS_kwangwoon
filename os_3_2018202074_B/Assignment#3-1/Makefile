CC = gcc

all: hello hello2 hello3

hello: fork.c
	$(CC) -o fork $^

hello2: numgen.c
	$(CC) -o numgen $^

hello3: thread.c
	$(CC) -o thread $^ -lpthread


clean:
	rm hello hello2 hello3
