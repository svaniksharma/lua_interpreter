#include "structs.h"
#include "error.h"
#include "debug.h"
#include "lua_string.h"

LUA_OBJ init_lua_obj(LUA_TYPE type, void *ptr) {
    LUA_OBJ o = { 0 };
    o.type = type;
    switch (o.type) {
        case REAL:
            o.value.n = * (LUA_REAL *) ptr;
            break;
        case BOOL:
            o.value.b = * (LUA_BOOL *) ptr;
            break;
        default:
            o.value.ptr = ptr;
            break;
    }
    return o;
}

LUA_OBJ init_lua_obj_val(LUA_TYPE type, LUA_VAL v) {
    LUA_OBJ o = { 0 };
    o.type = type;
    o.value = v;
    return o;
}

void make_lua_obj_cpy(LUA_OBJ *src, LUA_OBJ *dst) {
    if (src == NULL) {
        dst->type = NIL;
        return;
    }
    dst->type = src->type;
    switch (src->type) {
        case REAL:
            dst->value.b = 0;
            dst->value.ptr = NULL;
            dst->value.n = src->value.n;
            break;
        case BOOL:
            dst->value.n = 0;
            dst->value.ptr = NULL;
            dst->value.b = src->value.b;
            break;
        default:
            dst->value.n = 0;
            dst->value.b = 0;
            dst->value.ptr = src->value.ptr;
            break;
    }
}

#ifdef LUA_DEBUG

void print_lua_obj(LUA_OBJ *o) {
    switch (o->type) {
        case REAL:
            LOG_DEBUG("%g", o->value.n);
            break;
        case BOOL:
            LOG_DEBUG("%s", o->value.b == TRUE ? "true" : "false");
            break;
        case NIL:
            LOG_DEBUG("nil");
            break;
        case STR: {
#define MAX_STR_PRINT 100
            LUA_STR *str = (LUA_STR *) o->value.ptr;
            char buf[MAX_STR_PRINT+1] = { 0 };
            snprintf(buf, MAX_STR_PRINT+1, "%s", str->str);
            LOG_DEBUG("%s", buf);
#undef MAX_STR_PRINT
            break;
        }
        default:
            break;
    }
}

#endif

static LUA_BOOL resize_dyn_arr(uint8_t **arr, int size_each, int old_capacity, int new_capacity) {
    uint8_t *new_arr = realloc(*arr, size_each * new_capacity);
    CHECK(new_arr != NULL);
    if (new_arr != *arr)
        *arr = new_arr;
    return TRUE;
lua_err:
    return FALSE;
}

ERR init_dyn_arr(DYN_ARR *d, int size_each) {
    d->n = 0;
    d->size_each = size_each;
    d->capacity = INITIAL_CAP;
    return SAFE_ALLOC(&d->arr, d->size_each * d->capacity);
}

LUA_BOOL add_dyn_arr(DYN_ARR *d, uint8_t *item) {
    if (d->n == d->capacity) {
        int new_capacity = d->capacity * 2;
        if (!resize_dyn_arr(&d->arr, d->size_each, d->capacity, new_capacity))
            return FALSE;
        d->capacity = new_capacity;
    }
    memcpy(d->arr + d->n * d->size_each, item, d->size_each);
    ++d->n;
    return TRUE;
}

void destroy_dyn_arr(DYN_ARR *d) {
    d->n = 0;
    d->size_each = 0;
    d->capacity = 0;
    SAFE_FREE(&d->arr);
}

ERR safe_alloc(void **ptr, int size) {
    if (size <= 0) {
        *ptr = NULL;
        return SUCCESS;
    }
    if (*ptr == NULL) {
        void *alloc = calloc(1, size);
        if (!alloc)
            return ALLOC_ERR;
        *ptr = alloc;
        return SUCCESS;
    }
    return ALLOC_ERR;
}

void safe_free(void **ptr) {
    if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
}

