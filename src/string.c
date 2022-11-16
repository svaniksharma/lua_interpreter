#include "string.h"
#include "structs.h"

LUA_STR *init_lua_str(char *str, int size) {
    LUA_STR *s = NULL;
    if (SAFE_ALLOC(&s, sizeof(LUA_STR)) == ALLOC_ERR)
        return NULL;
    s->size = size + 1;
    if (SAFE_ALLOC(&s->str, s->size) == ALLOC_ERR) {
        SAFE_FREE(&s);
        return NULL;
    }
    strncpy(s->str, str, size);
    s->str[s->size - 1] = '\0';
    s->hash = str_hash(s->str);
    return s;
}

uint32_t str_obj_hash(void *key) {
    LUA_STR *str_obj = (LUA_STR *) key;
    char *str = str_obj->str;
    return str_hash(str);
}

uint32_t str_hash(void *key) {
    char *str = (char *) key;
    uint32_t hash = 2166136261u;
    for (int i = 0; str[i] != '\0'; i++) {
        hash ^= (uint8_t) str[i];
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
