#ifndef TABLE_H
#define TABLE_H

#include "structs.h"
#include "string.h"
#include <stdint.h>

#define TABLE_LOAD_FACTOR 0.75
#define INIT_TABLE_CAPACITY 10000

typedef uint32_t (*hash_func)(void *key);
typedef LUA_BOOL (*equals_func)(void *a, void *b);

typedef struct entry {
    void *key;
    LUA_OBJ value;
} ENTRY;

typedef struct table {
    int size;
    int capacity;
    hash_func hash;
    equals_func eq;
    ENTRY *entries;
} TABLE;

LUA_BOOL equals_ptr(void *a, void *b);
LUA_BOOL equals_str(void *a, void *b);

ERR init_table(TABLE *table, hash_func hash, equals_func eq);
int size_table(TABLE *table);
LUA_BOOL put_table(TABLE *table, void *key, void *value);
void *get_table(TABLE *table, void *key);
LUA_OBJ *get_table_str(TABLE *table, char *key, int size);
LUA_BOOL remove_table(TABLE *table, void *key);
void destroy_table(TABLE *table);
void destroy_table_and_keys(TABLE *table);

#endif
