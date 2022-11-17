#ifndef STRUCTS_H
#define STRUCTS_H

#include "error.h"
#include <stdint.h>

#define FALSE 0
#define TRUE 1

/* TYPES FOR LUA (and the interpreter itself) */

typedef uint8_t LUA_BOOL;
typedef double  LUA_REAL;

typedef enum lua_type {
    BOOL,
    REAL,
    NIL,
    STR,
} LUA_TYPE;

typedef union lua_val {
    LUA_BOOL b;
    LUA_REAL n;
    void *ptr;
} LUA_VAL;

typedef struct lua_obj {
    LUA_TYPE type;
    LUA_VAL value;
} LUA_OBJ;

LUA_OBJ init_lua_obj(LUA_TYPE type, void *ptr);
LUA_OBJ init_lua_obj_val(LUA_TYPE type, LUA_VAL v);
void make_lua_obj_cpy(LUA_OBJ *src, LUA_OBJ *dst);

#define IS_NIL(obj) ((obj).type == NIL)
#define IS_NUM(obj) ((obj).type == REAL)
#define IS_BOOL(obj) ((obj).type == BOOL || (obj).type == NIL)
#define IS_STR(obj) ((obj).type == STR)

#define AS_NIL(obj) "nil"
#define AS_NUM(obj) (obj).value.n
#define AS_BOOL(obj) (obj).value.b
#define AS_STR(obj) ((LUA_STR *) ((obj).value.ptr))

#ifdef LUA_DEBUG

void print_lua_obj(LUA_OBJ *obj);

#endif

/* DYNAMIC ARRAY */

#define INITIAL_CAP 10

typedef struct dyn_arr {
    int n;
    int size_each;
    int capacity;
    uint8_t *arr;
} DYN_ARR;

ERR init_dyn_arr(DYN_ARR *d, int size_each);
LUA_BOOL add_dyn_arr(DYN_ARR *d, uint8_t *item);
void destroy_dyn_arr(DYN_ARR *d);

#define SIZE_DYN_ARR(d) d.n
#define CAP_DYN_ARR(d) d.capacity
#define ADD_DYN_ARR(d, item) add_dyn_arr(d, (uint8_t *) (item))
#define GET_DYN_ARR(d, i, type) (* ((type *) (d.arr + (i) * sizeof(type))))

/* MEMORY ALLOCATION */

ERR safe_alloc(void **ptr, int size);
void safe_free(void **ptr);

// So we don't have to explicitly cast to (void **)
#define SAFE_ALLOC(ptr, size) safe_alloc((void **) (ptr), (size))
#define SAFE_FREE(ptr) safe_free((void **) (ptr))

#endif
