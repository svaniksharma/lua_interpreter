#include "debug.h"
#include "structs.h"
#include "lua_string.h"
#include <string.h>

LUA_STR *init_lua_str(char *str, int size) {
    LUA_STR *s = NULL;
    CHECK(SAFE_ALLOC(&s, sizeof(LUA_STR)) != ALLOC_ERR);
    s->size = size;
    CHECK(SAFE_ALLOC(&s->str, s->size + 1) != ALLOC_ERR);
    strncpy(s->str, str, s->size);
    s->str[s->size] = '\0';
    s->hash = str_hash_len(s->str, s->size);
    return s;
lua_err:
    SAFE_FREE(&s->str);
    SAFE_FREE(&s);
    return NULL;
}

LUA_STR *cat_lua_str(LUA_STR *a, LUA_STR *b) {
   LUA_STR *c = NULL;
   CHECK(SAFE_ALLOC(&c, sizeof(LUA_STR)) != ALLOC_ERR);
   c->size = a->size + b->size;
   CHECK(SAFE_ALLOC(&c->str, c->size + 1) != ALLOC_ERR);
   strncpy(c->str, a->str, a->size);
   strncat(c->str, b->str, b->size);
   c->str[c->size] = '\0';
   c->hash = str_hash_len(c->str, c->size);
   return c;
lua_err:
   SAFE_FREE(&c->str);
   SAFE_FREE(&c);
   return NULL;
}

uint32_t str_obj_hash(void *key) {
    LUA_STR *str_obj = (LUA_STR *) key;
    char *str = str_obj->str;
    return str_hash_len(str, str_obj->size);
}

uint32_t str_hash_len(char *key, int size) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < size; i++) {
        hash ^= (uint8_t) key[i];
        hash *= 16777619;
    }
    return hash;
}

void destroy_lua_str(LUA_STR **str) {
    LUA_STR *ptr = *str;
    if (ptr != NULL) {
        ptr->size = 0;
        ptr->hash = 0;
        SAFE_FREE(&ptr->str);
        SAFE_FREE(&ptr);
    }
}
