#include "debug.h"
#include "structs.h"
#include "vm.h"

static void push_vm_stack(LUA_VM *vm, LUA_OBJ v) {
    *vm->top = v;
    ++vm->top;
}

static LUA_OBJ pop_vm_stack(LUA_VM *vm) {
    vm->top--;
    return *vm->top;
}

void init_vm(LUA_VM *vm) {
    vm->curr_chunk = NULL;
    vm->ip = NULL;
    SAFE_ALLOC(&vm->stack, DEFAULT_STACK_SIZE * sizeof(LUA_OBJ));
    vm->top = vm->stack;
}

void run_vm(LUA_VM *vm, LUA_CHUNK *chunk) {
    vm->curr_chunk = chunk;
    vm->ip = chunk->code.arr;
    while (TRUE) {
        LUA_OPCODE opcode = *vm->ip++;
        switch (opcode) {
            case OP_CONST: {
                int index = *vm->ip++;
                LUA_OBJ o = { 0 };
                o.type = REAL;
                o.value = GET_DYN_ARR(chunk->values, index, LUA_VAL);
                push_vm_stack(vm, o);
                break;
            }
            case OP_NEGATE: {
                LUA_REAL r = AS_NUM(pop_vm_stack(vm));
                LUA_VAL v = { 0 };
                v.n = -r;
                LUA_OBJ o = { REAL, v };
                push_vm_stack(vm, o);
                break;
            }
            case OP_ADD: PERFORM_NUM_BINARY_OP(+);
            case OP_SUB: PERFORM_NUM_BINARY_OP(-);
            case OP_MULT: PERFORM_NUM_BINARY_OP(*);
            case OP_DIV: PERFORM_NUM_BINARY_OP(/);
            case OP_EXP:
                break;
            case OP_CAT:
                break;
            case OP_RETURN: {
                LUA_OBJ o = pop_vm_stack(vm);
#ifdef LUA_DEBUG
                print_lua_obj(&o);
#endif
                return;
            }
        }
    }
}

void destroy_vm(LUA_VM *vm) {
    vm->ip = NULL;
    vm->top = NULL;
    vm->curr_chunk = NULL;
    SAFE_FREE(&vm->stack);
}
