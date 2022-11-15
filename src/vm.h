#ifndef LUA_VM_H
#define LUA_VM_H

#include "chunk.h"

#define PERFORM_NUM_BINARY_OP(op) \
{ \
    LUA_OBJ second_obj = pop_vm_stack(vm); \
    LUA_OBJ first_obj = pop_vm_stack(vm); \
    if (!IS_NUM(first_obj) || !IS_NUM(second_obj)) { \
        report_runtime_err("Expected number"); \
        return; \
    } \
    LUA_REAL second = AS_NUM(second_obj); \
    LUA_REAL first = AS_NUM(first_obj); \
    LUA_REAL res = first op second; \
    LUA_OBJ obj = init_lua_obj(REAL, &res); \
    push_vm_stack(vm, obj); \
    break; \
}

#define PERFORM_BOOL_BINARY_OP(op) \
{ \
    LUA_OBJ second_obj = pop_vm_stack(vm); \
    LUA_OBJ first_obj = pop_vm_stack(vm); \
    if (!IS_NUM(first_obj) || !IS_NUM(second_obj)) { \
        report_runtime_err("Expected number"); \
        return; \
    } \
    LUA_REAL second = AS_NUM(second_obj); \
    LUA_REAL first = AS_NUM(first_obj); \
    LUA_BOOL res = first op second; \
    LUA_OBJ obj = init_lua_obj(BOOL, &res); \
    push_vm_stack(vm, obj); \
    break; \
}

#define DEFAULT_STACK_SIZE 1024

typedef struct lua_vm {
    uint8_t *ip; // points inside of the current chunk's code
    LUA_CHUNK *curr_chunk;
    LUA_OBJ stack[DEFAULT_STACK_SIZE];
    LUA_OBJ *top;
} LUA_VM;

void init_vm(LUA_VM *vm);
void run_vm(LUA_VM *vm, LUA_CHUNK *chunk);
void destroy_vm(LUA_VM *vm);

#endif
