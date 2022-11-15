#include "debug.h"
#include "structs.h"
#include "vm.h"
#include <math.h>

static void report_runtime_err(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}


static void push_vm_stack(LUA_VM *vm, LUA_OBJ v) {
    *vm->top = v;
    ++vm->top;
}

static LUA_OBJ pop_vm_stack(LUA_VM *vm) {
    vm->top--;
    return *vm->top;
}

static LUA_OBJ peek_vm_stack(LUA_VM *vm) {
    return *(vm->top - 1);
}

static LUA_BOOL equal_objs(LUA_OBJ *a, LUA_OBJ *b) {
    if (a->type != b->type)
        return FALSE;
    switch (a->type) {
        case BOOL: return a->value.b == b->value.b;
        case REAL: return a->value.n == b->value.n;
        default: return FALSE;
    }
}

void init_vm(LUA_VM *vm) {
    vm->curr_chunk = NULL;
    vm->ip = NULL;
    memset(vm->stack, 0, sizeof(vm->stack));
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
                LUA_OBJ o = GET_DYN_ARR(chunk->values, index, LUA_OBJ);
                push_vm_stack(vm, o);
                break;
            }
            case OP_NEGATE: {
                if (!IS_NUM(peek_vm_stack(vm))) {
                    report_runtime_err("Expected real number");
                    return;
                }
                LUA_REAL r = -AS_NUM(pop_vm_stack(vm));
                LUA_OBJ o = init_lua_obj(REAL, &r);
                push_vm_stack(vm, o);
                break;
            }
            case OP_ADD: PERFORM_NUM_BINARY_OP(+);
            case OP_SUB: PERFORM_NUM_BINARY_OP(-);
            case OP_MULT: PERFORM_NUM_BINARY_OP(*);
            case OP_DIV: PERFORM_NUM_BINARY_OP(/);
            case OP_EXP: {
                LUA_OBJ second_obj = pop_vm_stack(vm);
                LUA_OBJ first_obj = pop_vm_stack(vm);
                if (!IS_NUM(first_obj) || !IS_NUM(second_obj)) {
                    report_runtime_err("Expected number");
                    return;
                }
                LUA_REAL second = AS_NUM(second_obj);
                LUA_REAL first = AS_NUM(first_obj);
                LUA_REAL res = pow(first, second);
                LUA_OBJ obj = init_lua_obj(REAL, &res);
                push_vm_stack(vm, obj);
                break;
            }
            case OP_EQ: {
                LUA_OBJ second_obj = pop_vm_stack(vm);
                LUA_OBJ first_obj = pop_vm_stack(vm);
                LUA_BOOL b = equal_objs(&first_obj, &second_obj);
                push_vm_stack(vm, init_lua_obj(BOOL, &b));
                break;
            }
            case OP_LE: PERFORM_BOOL_BINARY_OP(<=);
            case OP_LT: PERFORM_BOOL_BINARY_OP(<);
            case OP_GE: PERFORM_BOOL_BINARY_OP(>=);
            case OP_GT: PERFORM_BOOL_BINARY_OP(>);
            case OP_NOT: {
                if (!IS_BOOL(peek_vm_stack(vm))) {
                    report_runtime_err("Expected boolean");
                    return;
                }
                LUA_BOOL b = !AS_BOOL(pop_vm_stack(vm));
                LUA_OBJ o = init_lua_obj(BOOL, &b);
                push_vm_stack(vm, o);
                break;
            }
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
}
