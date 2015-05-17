CC = clang

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

CFLAGS = -Isrc/ -O3 -g -Wall -Wextra

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
