build:
	gcc -o bin/server -Wall -pedantic src/server_select.c src/hashtable.c

clean:
	rm bin/server
