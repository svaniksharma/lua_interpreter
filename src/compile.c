#include "debug.h"
#include "vm.h"
#include "compile.h"
#include "lexer.h"
#include <stdio.h>

void run(char *source, int length) {
    LUA_CHUNK chunk = { 0 };
    init_chunk(&chunk);
    LUA_VAL v = { 0 };
    v.n = 1.2;
    write_const_chunk(&chunk, &v);
    write_byte_chunk(&chunk, OP_CONST);
    write_byte_chunk(&chunk, 0);
    write_byte_chunk(&chunk, OP_NEGATE);
    write_byte_chunk(&chunk, OP_RETURN);
    LUA_VM vm = { 0 };
    init_vm(&vm);
    run_vm(&vm, &chunk);
    destroy_chunk(&chunk);
    destroy_vm(&vm);
}
