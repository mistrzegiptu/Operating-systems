CC = gcc
CFLAGS = -Wall -Wextra -std=c17

all: task1 client program

task1:
	$(CC) $(CFLAGS) -o task1 task1.c

client:
	$(CC) $(CFLAGS) -o client client.c

program:
	$(CC) $(CFLAGS) -o program program.c

clean:
	rm -f task1 client program

.PHONY: all task1 client program