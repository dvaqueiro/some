all: build repl

build:
	gcc -o bin/some_server -g -Wall -pedantic -fsanitize=address src/some.c src/server.c src/hashtable.c src/command.c

repl:
	gcc -o bin/repl -g -Wall -pedantic -fsanitize=address src/repl.c -lreadline -ltermcap

clean:
	rm bin/some_server
