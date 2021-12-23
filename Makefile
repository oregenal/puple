CC=gcc
CFLAGS=-Wall -Werror -ftrapv -std=c11 -pedantic -ggdb
LIBS=
PKGS=libpulse-simple

BIN=puple
OBJDIR=obj
SRCDIR=src

HDRS=$(wildcard $(SRCDIR)/*.h)
OBJS=$(patsubst $(SRCDIR)/%.h, $(OBJDIR)/%.o, $(HDRS))

.PHONY:default clean

default:$(BIN)

$(OBJDIR)/%.o:$(SRCDIR)/%.c $(SRCDIR)/%.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN):$(SRCDIR)/main.c $(OBJS)
	$(CC) $(CFLAGS) `pkg-config --cflags $(PKGS)` -o $@ $^ $(LIBS) `pkg-config --libs $(PKGS)`

$(OBJDIR):
	@mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BIN)
