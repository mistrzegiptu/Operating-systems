CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -pthread -lrt

all: main

main:
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -f main

.PHONY: all main clean