CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g -fsanitize=address

all: task1 task2

task1:
	$(CC) $(CFLAGS) -o task1 task1.c

task2:
	$(CC) $(CFLAGS) -o task2 task2.c

clean:
	rm -f task1 task2

.PHONY: all task1 task2 clean
