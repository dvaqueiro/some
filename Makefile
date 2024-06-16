build:
	gcc -o bin/repl -Wall -pedantic src/repl.c src/hashtable.c

clean:
	rm bin/repl
