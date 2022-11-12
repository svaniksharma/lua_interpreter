CFLAGS=-std=c99
DEBUG_FLAGS=-g -Wall -DLUA_DEBUG
SRC=*.c

all:
	cc $(CFLAGS) $(DEBUG_FLAGS) $(SRC) -o main
clean:
	- rm -rf main *.dSYM
