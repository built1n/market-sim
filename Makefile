CC = cc
INSTALL = install

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

GIT_VERSION := $(shell git describe --abbrev=8 --always --dirty)

CFLAGS = -Isrc/ -O2 -g -Wall -Wextra -std=gnu99 -DVERSION_INFO=\"$(GIT_VERSION)\"

PREFIX = /usr
BINDIR = $(PREFIX)/bin

LIBS = -lcurl -lcurses

HEADERS := $(wildcard src/*.h)


market-sim: $(OBJ) Makefile $(HEADERS)
	@echo "LD $@"
	@$(CC) $(OBJ) -o $@ $(CFLAGS) $(LIBS)

%.o: %.c Makefile $(HEADERS)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

install: all
	@echo "INSTALL market-sim"
	@install market-sim $(BINDIR)

clean:
	@echo "Cleaning build directory..."
	@rm -f $(OBJ) market-sim

all: market-sim
