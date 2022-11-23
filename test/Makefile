SRC=../src/table.c ../src/structs.c ../src/lua_string.c
INCLUDE=-I ../src
CFLAGS=-std=c99 -Wall -Werror -g
%_test: %_test.c
	cc $(INCLUDE) $(SRC) $(CFLAGS) $^ -o $@
clean:
	- rm -rf *_test *.dSYM
