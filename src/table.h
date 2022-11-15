#ifndef TABLE_H
#define TABLE_H

#include "structs.h"
#include <stdint.h>

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
char *get_table_str(TABLE *table, char *str);
LUA_BOOL remove_table(TABLE *table, void *key);
void destroy_table(TABLE *table);

uint32_t str_hash(void *key);

#endif
