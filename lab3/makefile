CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g -fsanitize=address

all: flipper

flipper:
	$(CC) $(CFLAGS) -o flipper flipper.c

clean:
	rm -f flipper

.PHONY: all flipper clean