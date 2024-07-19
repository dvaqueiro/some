# SoMe. Save Objects in Memory

This is a exploratory project that allows you to save objects in memory. Is a very basic aproach of best know REDIS.

The idea is create a client-server architecture where the server is responsible to save the objects in memory and the client is responsible to send the objects to the server and retrieve them.

The only purpose of this project is to learn how to create a client-server architecture and how to save objects in memory.

## Primary Goals

- Repl (Read-Eval-Print Loop) is a simple interactive programming environment that takes single user inputs, executes them, and returns the result to the user; it then repeats this process.
- Single threaded TCP server that listens for multiple clients.
- Event loop to handle multiple request in a single thread.
- Hashmap to save and restore objects by keys.

## TODO
- [ ] Increase hash table size when it reaches a certain threshold.
- [X] repl allow history commands.
- [X] repl allow enquoted strings.

## Build and run

To build
```bash
$ make all
```

To run the repl
```bash
$ ./bin/repl
```

To run the server
```bash
$ ./bin/some_server
```
