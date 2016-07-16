CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -fwrapv -I$(TERMIO_PREFIX)/include -I$(POSTL_PREFIX)/include

BIN = editor

TERMIO_PREFIX = $(HOME)/prefix
POSTL_PREFIX = $(HOME)/prefix

.PHONY: all clean remake

all: $(BIN)

clean:
	rm -rf $(BIN) *.dSYM

remake: clean all


$(BIN): $(wildcard *.c *.h)
	$(CC) $(CFLAGS) -o $@ $(filter-out %.h,$^) $(TERMIO_PREFIX)/lib/libtermio.a $(POSTL_PREFIX)/lib/libpostl.a
