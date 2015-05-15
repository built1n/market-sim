CC = cc

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

CFLAGS = -Isrc/ -O3 -std=c99 -g -Wall -lcurl -fsanitize=address

HEADERS := $(wildcard src/*.h)

market-sim: $(OBJ) Makefile $(HEADERS)
	@echo "LD $@"
	@$(CC) $(OBJ) -o $@ $(CFLAGS)

%.o: %.c Makefile $(HEADERS)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build directory..."
	@rm -f $(OBJ) market-sim

all:
	@echo $(SRC) $(OBJ)
