CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: server cliente

server: server.c
	$(CC) $(CFLAGS) server.c -o server

cliente: cliente.c
	$(CC) $(CFLAGS) cliente.c -o cliente

clean:
	rm -f server cliente

.PHONY: all clean
