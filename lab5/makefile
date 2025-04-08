CC = gcc
CFLAGS = -Wall -Wextra -std=c17

all: task1 sender catcher

task1:
	$(CC) $(CFLAGS) -o task1 task1.c

sender:
	$(CC) $(CFLAGS) -o sender sender.c

catcher:
	$(CC) $(CFLAGS) -o catcher catcher.c

clean:
	rm -f task1 sender catcher

.PHONY: all task1 sender catcher clean
