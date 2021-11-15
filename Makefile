CC=gcc
CFLAGS=-Wall -Werror -std=c11 -pedantic
LIBS=-lpulse-simple
BIN=pupl
PKGS=libpulse-simple

$(BIN):main.c
	$(CC) $(CFLAGS) `pkg-config --cflags $(PKGS)` \
		-o $(BIN) $< $(LIBS) `pkg-config --libs $(PKGS)`
