SRC=../src/error.c ../src/structs.c ../src/table.c
INCLUDE=-I ../src
CFLAGS=-Wall -Werror -g
%_test: %_test.c
	cc $(INCLUDE) $(SRC) $(CFLAGS) $^ -o $@
clean:
	- rm -rf *_test *.dSYM
