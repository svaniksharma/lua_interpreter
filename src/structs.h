#ifndef STRUCTS_H
#define STRUCTS_H

#include "error.h"
#include <stdint.h>

#define FALSE 0
#define TRUE 1

/* TYPES FOR LUA (and the interpreter itself) */

typedef int    LUA_BOOL;
typedef double LUA_REAL;

typedef enum lua_type {
    BOOL,
    REAL,
    STR,
} LUA_TYPE;

typedef struct lua_str {
    int size;
    char *str;
    uint32_t hash;
} LUA_STR;

typedef union lua_val {
    LUA_BOOL b;
    LUA_REAL n;
    void *ptr;
} LUA_VAL;

typedef struct lua_obj {
    LUA_TYPE type;
    LUA_VAL *value;
} LUA_OBJ;

/* MEMORY ALLOCATION */

ERR safe_alloc(void **ptr, int size);
void safe_free(void **ptr);

// So we don't have to explicitly cast to (void **)
#define SAFE_ALLOC(ptr, size) safe_alloc((void **) (ptr), (size))
#define SAFE_FREE(ptr) safe_free((void **) (ptr))

#endif
