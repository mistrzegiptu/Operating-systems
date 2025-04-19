CC = gcc
CFLAGS = -Wall -Wextra -std=c17

all: client server

client:
	$(CC) $(CFLAGS) -o client client.c

server:
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f task1 client server

.PHONY: all client server