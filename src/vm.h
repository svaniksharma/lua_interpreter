#ifndef LUA_VM_H
#define LUA_VM_H

#include "chunk.h"

#define PERFORM_NUM_BINARY_OP(op) \
{ \
    LUA_REAL second = AS_NUM(pop_vm_stack(vm)); \
    LUA_REAL first = AS_NUM(pop_vm_stack(vm)); \
    LUA_VAL v = { 0 }; \
    v.n = first op second; \
    LUA_OBJ obj = {REAL, v}; \
    push_vm_stack(vm, obj); \
    break; \
}

#define DEFAULT_STACK_SIZE 1024

typedef struct lua_vm {
    uint8_t *ip; // points inside of the current chunk's code
    LUA_CHUNK *curr_chunk;
    LUA_OBJ *stack;
    LUA_OBJ *top;
} LUA_VM;

void init_vm(LUA_VM *vm);
void run_vm(LUA_VM *vm, LUA_CHUNK *chunk);
void destroy_vm(LUA_VM *vm);

#endif
