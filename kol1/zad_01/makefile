CC = gcc
CFLAGS = -Wall -Wextra -std=c17
LDFLAGS = -ldl
LIBDIR = ./lib
BINDIR = ./bin
SRCDIR = .

.PHONY: all clean

all: $(BINDIR)/zad1 $(LIBDIR)/bibl1.so

$(LIBDIR)/bibl1.so: $(LIBDIR)/bibl1.o
	$(CC) -shared -o $@ $^

$(BINDIR)/zad1: $(SRCDIR)/zad1.c | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(LIBDIR)/bibl1.o: $(SRCDIR)/bibl1.c $(SRCDIR)/bibl1.h | $(LIBDIR)
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(BINDIR) $(LIBDIR):
	mkdir -p $@

clean:
	rm -f $(LIBDIR)/*.o $(LIBDIR)/*.so $(BINDIR)/zad1
	rmdir --ignore-fail-on-non-empty $(BINDIR) $(LIBDIR)