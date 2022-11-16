#ifndef LUA_VM_H
#define LUA_VM_H

#include "chunk.h"
#include "table.h"

#define PERFORM_BINARY_OP_WITH_NUM(op, decl, type) \
{ \
    LUA_OBJ second_obj = pop_vm_stack(vm); \
    LUA_OBJ first_obj = pop_vm_stack(vm); \
    if (!IS_NUM(first_obj) || !IS_NUM(second_obj)) { \
        report_runtime_err("Expected number"); \
        return; \
    } \
    LUA_REAL second = AS_NUM(second_obj); \
    LUA_REAL first = AS_NUM(first_obj); \
    decl res = first op second; \
    LUA_OBJ obj = init_lua_obj(type, &res); \
    push_vm_stack(vm, obj); \
    break; \
}

#define PERFORM_BINARY_OP_WITH_BOOL(op) \
{ \
    LUA_OBJ second_obj = pop_vm_stack(vm); \
    LUA_OBJ first_obj = pop_vm_stack(vm); \
    if (!IS_BOOL(first_obj) || !IS_BOOL(second_obj)) { \
        report_runtime_err("Expected boolean"); \
        return; \
    } \
    LUA_BOOL second = AS_BOOL(second_obj); \
    LUA_BOOL first = AS_BOOL(first_obj); \
    LUA_BOOL res = first op second; \
    LUA_OBJ obj = init_lua_obj(BOOL, &res); \
    push_vm_stack(vm, obj); \
    break; \
}


#define DEFAULT_STACK_SIZE 1024

typedef struct lua_vm {
    uint8_t *ip; // points inside of the current chunk's code
    TABLE strings;
    LUA_CHUNK *curr_chunk;
    LUA_OBJ stack[DEFAULT_STACK_SIZE];
    LUA_OBJ *top;
} LUA_VM;

void init_vm(LUA_VM *vm);
void run_vm(LUA_VM *vm, LUA_CHUNK *chunk);
void destroy_vm(LUA_VM *vm);

#endif
