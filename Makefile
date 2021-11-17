CC=gcc
CFLAGS=-Wall -Werror -std=c11 -pedantic -ggdb
LIBS=-lpulse-simple
BIN=puple
PKGS=libpulse-simple

$(BIN):main.c
	$(CC) $(CFLAGS) `pkg-config --cflags $(PKGS)` -o $(BIN) $< $(LIBS) `pkg-config --libs $(PKGS)`
