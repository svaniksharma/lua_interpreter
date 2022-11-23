#ifndef LUA_STR_H
#define LUA_STR_H

#include <stdint.h>

typedef struct lua_str {
    int size;
    char *str;
    uint32_t hash;
} LUA_STR;

LUA_STR *init_lua_str(char *str, int size);
LUA_STR *cat_lua_str(LUA_STR *a, LUA_STR *b);
uint32_t str_obj_hash(void *key);
uint32_t str_hash_len(char *key, int size);
uint32_t str_hash(void *key);
void destroy_lua_str_heap(LUA_STR **str);
void destroy_lua_str(LUA_STR **str);

#endif
