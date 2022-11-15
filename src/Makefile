CFLAGS=-std=c99
DEBUG_FLAGS=-g -Wall -DLUA_DEBUG
SRC=*.c
LIB=-lm

all:
	cc $(CFLAGS) $(DEBUG_FLAGS) $(SRC) $(LIB) -o main
clean:
	- rm -rf main *.dSYM
