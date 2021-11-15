CC=gcc
CFLAGS=-Wall -Werror -std=c11 -pedantic
LIBS=-lpulse-simple
BIN=pupl

$(BIN):main.c
	$(CC) $(CFLAGS) -o $(BIN) $< $(LIBS)
