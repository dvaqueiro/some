all: build repl

build:
	gcc -o bin/some_server -Wall -pedantic src/some.c src/server.c src/hashtable.c src/command.c

repl:
	gcc -o bin/repl -Wall -pedantic src/repl.c

clean:
	rm bin/some_server
