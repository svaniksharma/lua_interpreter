CFLAGS=-std=c99 -Wall -Wpedantic -pedantic-errors -Wunused -Wextra
DEBUG_FLAGS=-g -DLUA_DEBUG
SRC=*.c
LIB=-lm

all:
	cc $(CFLAGS) $(DEBUG_FLAGS) $(SRC) $(LIB) -o main
clean:
	- rm -rf main *.dSYM
