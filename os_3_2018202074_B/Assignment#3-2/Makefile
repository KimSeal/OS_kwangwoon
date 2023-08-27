all: hello hello2

hello: schedtest.c
	$(CC) -o schedtest $^

hello2: filegen.c
	$(CC) -o filegen $^

clean:
	rm hello | rm hello2
