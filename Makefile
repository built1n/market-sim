CC = cc

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

GIT_VERSION := $(shell git describe --abbrev=8 --dirty --always --tags)

CFLAGS = -Isrc/ -O2 -g -Wall -Wextra -std=gnu99 -fsanitize=address -DVERSION_INFO=\"$(GIT_VERSION)\"

HEADERS := $(wildcard src/*.h)

market-sim: $(OBJ) Makefile $(HEADERS)
	@echo "LD $@"
	@$(CC) $(OBJ) -o $@ $(CFLAGS) -lcurl

%.o: %.c Makefile $(HEADERS)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build directory..."
	@rm -f $(OBJ) market-sim

all:
	@echo $(SRC) $(OBJ)
