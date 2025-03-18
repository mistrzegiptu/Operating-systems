CC = gcc
CFLAGS = -Wall -Wextra -std=c17
LDFLAGS = -ldl

all: libcollatz.a client_static libcollatz.so client_shared client_dynamic

libcollatz.a: collatz.o
	ar rcs libcollatz.a collatz.o

libcollatz.so: collatz.o
	$(CC) -shared -o libcollatz.so collatz.o

client_static: client.c libcollatz.a
	$(CC) $(CFLAGS) -o client_static client.c -L. -lcollatz

client_shared: client.c libcollatz.so
	$(CC) $(CFLAGS) -o client_shared client.c -L. -lcollatz -Wl,-rpath,.

client_dynamic: client.c
	$(CC) $(CFLAGS) -DDYNAMIC_LOAD -o client_dynamic client.c $(LDFLAGS)

collatz.o: collatz.c collatz.h
	$(CC) $(CFLAGS) -fPIC -c collatz.c

clean:
	rm -f *.o *.a *.so client_static client_shared client_dynamic

.PHONY: all clean
