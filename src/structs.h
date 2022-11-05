#ifndef STRUCTS_H
#define STRUCTS_H

#include "error.h"
#include <stdint.h>

#define FALSE 0
#define TRUE 1

/* TYPES FOR LUA (and the interpreter itself) */

typedef int    LUA_BOOL;
typedef double LUA_REAL;

/* MEMORY ALLOCATION */

ERR safe_alloc(void **ptr, int size);
void safe_free(void **ptr);

// So we don't have to explicitly cast to (void **)
#define SAFE_ALLOC(ptr, size) safe_alloc((void **) (ptr), (size))
#define SAFE_FREE(ptr) safe_free((void **) (ptr))

/* HASH TABLE */

#define TABLE_LOAD_FACTOR 0.75
#define INIT_TABLE_CAPACITY 10000

typedef uint32_t (*hash_func)(void *key);

typedef struct entry {
    void *key;
    void *value;
} ENTRY;

typedef struct table {
    int size;
    int capacity;
    hash_func hash;
    ENTRY *entries;
} TABLE;

ERR init_table(TABLE *table, hash_func hash);
int size_table(TABLE *table);
LUA_BOOL put_table(TABLE *table, void *key, void *value);
void *get_table(TABLE *table, void *key);
LUA_BOOL remove_table(TABLE *table, void *key);
void destroy_table(TABLE *table);

uint32_t str_hash(void *key);

#endif
