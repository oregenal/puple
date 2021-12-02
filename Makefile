CC=gcc
CFLAGS=-Wall -Werror -std=c11 -pedantic -ggdb
LIBS=-lpulse-simple
BIN=puple
PKGS=libpulse-simple
OBJS=str_search_ptrn.o
HEADERS=str_search_ptrn.h
OBJDIR=obj
SRCDIR=src

$(OBJDIR)/%.o:$(SRCDIR)/%.c $(SRCDIR)/$(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN):$(SRCDIR)/main.c $(OBJDIR)/$(OBJS) $(SRCDIR)/$(HEADERS)
	$(CC) $(CFLAGS) `pkg-config --cflags $(PKGS)` -o $(BIN) $< $(LIBS) $(OBJDIR)/$(OBJS) `pkg-config --libs $(PKGS)`

$(OBJDIR):
	@mkdir -p $@
