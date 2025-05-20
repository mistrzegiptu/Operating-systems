CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g

all: main

main:
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -f main


test: main
	@echo "Testing with different numbers of threads:\n"
	@for t in 1 2 4 8; do \
		echo "Threads: $$t"; \
		time ./main 0.00000001 $$t; \
		echo "\n"; \
	done

.PHONY: all main test clean
