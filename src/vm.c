#include "debug.h"
#include "structs.h"
#include "table.h"
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

static void reset_vm_stack(LUA_VM *vm) {
    vm->top = vm->stack;
}

#ifdef LUA_DEBUG

static void print_vm_instr(LUA_VM *vm) {
    uint8_t *ptr = vm->curr_chunk->code.arr;
    uint8_t *end_arr = vm->curr_chunk->code.arr + SIZE_DYN_ARR(vm->curr_chunk->code);
    printf("VM instructions:\n");
    while (ptr < end_arr) {
        printf("%s ", opcode_str_debug_table[*ptr]);
        switch (*ptr) {
            case OP_CONST:
            case OP_GET_GLOBAL:
            case OP_DEF_GLOBAL:
            case OP_SET_GLOBAL:
               printf("%d ", *(++ptr));
               break;
        }
        if (ptr == vm->ip-1)
            printf("| ");
        ++ptr;
    }
    printf("\n");
}

#endif

static LUA_BOOL equal_objs(LUA_OBJ *a, LUA_OBJ *b, TABLE *str_table) {
    if (a->type != b->type)
        return FALSE;
    switch (a->type) {
        case BOOL: return a->value.b == b->value.b;
        case REAL: return a->value.n == b->value.n;
        case STR:  {
            LUA_STR *a_str = (LUA_STR *) a->value.ptr;
            LUA_STR *b_str = (LUA_STR *) b->value.ptr;
            return a_str == b_str;
        }
        case NIL: return TRUE;
        default: return FALSE;
    }
    return FALSE; // unreachable
}

static ERR str_cat(LUA_VM *vm) {
    LUA_OBJ second_obj = pop_vm_stack(vm);
    LUA_OBJ first_obj = pop_vm_stack(vm);
    if (!IS_STR(second_obj) || !IS_STR(first_obj)) {
        report_runtime_err("Expected string");
        return FAIL;
    }
    LUA_STR *b = AS_STR(second_obj);
    LUA_STR *a = AS_STR(first_obj);
    char *key = NULL;
    LUA_STR *str = NULL;
    CHECK(SAFE_ALLOC(&key, a->size + b->size + 1) != ALLOC_ERR);
    strncpy(key, a->str, a->size);
    strncat(key, b->str, b->size);
    key[a->size + b->size] = '\0';
    LUA_OBJ *str_obj = get_table_str(&vm->strings, key, a->size + b->size);
    if (IS_NIL(*str_obj)) {
        CHECK(SAFE_ALLOC(&str, sizeof(LUA_STR)) != ALLOC_ERR);
        str->size = a->size + b->size;
        str->str = key;
        str->hash = str_hash(key);
        put_table(&vm->strings, str, NULL);
    } else {
        str = AS_STR(*str_obj);
    }
    LUA_OBJ obj = init_lua_obj(STR, str);
    push_vm_stack(vm, obj);
    return SUCCESS;
lua_err:
    SAFE_FREE(&key);
    SAFE_FREE(&str);
    return FAIL;
}

void init_vm(LUA_VM *vm) {
    vm->curr_chunk = NULL;
    vm->ip = NULL;
    init_table(&vm->strings, str_obj_hash);
    init_table(&vm->globals, str_obj_hash);
    memset(vm->stack, 0, sizeof(vm->stack));
    vm->top = vm->stack;
}

void run_vm(LUA_VM *vm, LUA_CHUNK *chunk) {
    vm->curr_chunk = chunk;
    vm->ip = chunk->code.arr;
    while (TRUE) {
#ifdef LUA_DEBUG
        print_vm_instr(vm);
#endif
        LUA_OPCODE opcode = *vm->ip++;
        switch (opcode) {
            case OP_POP: 
#ifndef LUA_DEBUG
                pop_vm_stack(vm); 
#else
                {
                    LUA_OBJ obj = pop_vm_stack(vm);
                    print_lua_obj(&obj);
                }
#endif
                break;
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
            case OP_ADD: PERFORM_BINARY_OP_WITH_NUM(+, LUA_REAL, REAL);
            case OP_SUB: PERFORM_BINARY_OP_WITH_NUM(-, LUA_REAL, REAL);
            case OP_MULT: PERFORM_BINARY_OP_WITH_NUM(*, LUA_REAL, REAL);
            case OP_DIV: PERFORM_BINARY_OP_WITH_NUM(/, LUA_REAL, REAL);
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
            case OP_EQ:
            case OP_NE: {
                LUA_OBJ second_obj = pop_vm_stack(vm);
                LUA_OBJ first_obj = pop_vm_stack(vm);
                LUA_BOOL b = equal_objs(&first_obj, &second_obj, &vm->strings);
                b = (opcode == OP_EQ) ? b : !b;
                push_vm_stack(vm, init_lua_obj(BOOL, &b));
                break;
            }
            case OP_LE: PERFORM_BINARY_OP_WITH_NUM(<=, LUA_BOOL, BOOL);
            case OP_LT: PERFORM_BINARY_OP_WITH_NUM(<, LUA_BOOL, BOOL);
            case OP_GE: PERFORM_BINARY_OP_WITH_NUM(>=, LUA_BOOL, BOOL);
            case OP_GT: PERFORM_BINARY_OP_WITH_NUM(>, LUA_BOOL, BOOL);
            case OP_NOT: {
                if (!IS_BOOL(peek_vm_stack(vm))) {
                    report_runtime_err("Expected boolean");
                    reset_vm_stack(vm);
                    return;
                }
                LUA_BOOL b = !AS_BOOL(pop_vm_stack(vm));
                LUA_OBJ o = init_lua_obj(BOOL, &b);
                push_vm_stack(vm, o);
                break;
            }
            case OP_AND: PERFORM_BINARY_OP_WITH_BOOL(&&);
            case OP_OR: PERFORM_BINARY_OP_WITH_BOOL(||);
            case OP_TRUE:
            case OP_FALSE: {
                LUA_BOOL b = (opcode == OP_TRUE) ? TRUE : FALSE;
                push_vm_stack(vm, init_lua_obj(BOOL, &b));
                break;
            }
            case OP_NIL:
                push_vm_stack(vm, init_lua_obj(NIL, NULL));
                break;
            case OP_CAT:
                if (str_cat(vm) != SUCCESS) {
                    reset_vm_stack(vm);
                    return;
                }
                break;
            case OP_DEF_GLOBAL: {
                int index = *vm->ip++;
                LUA_OBJ name = GET_DYN_ARR(chunk->values, index, LUA_OBJ);
                LUA_STR *name_str = AS_STR(name);
                LUA_OBJ name_val = peek_vm_stack(vm);
                put_table(&vm->globals, name_str, &name_val);
                pop_vm_stack(vm);
                break;
            }
            case OP_GET_GLOBAL: {
                int index = *vm->ip++;
                LUA_OBJ name = GET_DYN_ARR(chunk->values, index, LUA_OBJ);
                LUA_STR *name_str = AS_STR(name);
                LUA_OBJ *name_val = get_table_str(&vm->globals, name_str->str, name_str->size);
                if (IS_NIL(*name_val)) {
                    report_runtime_err("Undefined variable");
                    return;
                }
                push_vm_stack(vm, *name_val);
                break;
            }
            case OP_SET_GLOBAL: {
                int index = *vm->ip++;
                LUA_OBJ name = GET_DYN_ARR(chunk->values, index, LUA_OBJ);
                LUA_STR *name_str = AS_STR(name);
                LUA_OBJ name_val = peek_vm_stack(vm);
                LUA_OBJ *orig_name_val = get_table_str(&vm->globals, name_str->str, name_str->size);
                make_lua_obj_cpy(&name_val, orig_name_val);
                break;
            }
            case OP_RETURN:
                return;
        }
    }
}

void destroy_vm(LUA_VM *vm) {
    vm->ip = NULL;
    vm->top = NULL;
    vm->curr_chunk = NULL;
    destroy_table_and_keys(&vm->strings);
    destroy_table_and_keys(&vm->globals);
}
