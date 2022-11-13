#include "structs.h"
#include "error.h"
#include "debug.h"
#include <stdlib.h>
#include <string.h>

void print_lua_obj(LUA_OBJ *o) {
    switch (o->type) {
        case REAL:
            LOG_DEBUG("%g", o->value.n);
            break;
        case BOOL:
            LOG_DEBUG("%s", o->value.b == TRUE ? "true" : "false");
            break;
        default:
            break;
    }
}

static LUA_BOOL resize_dyn_arr(uint8_t **arr, int size_each, int old_capacity, int new_capacity) {
    TRY {
        uint8_t *new_arr = realloc(*arr, size_each * new_capacity);
        THROW(new_arr == NULL);
        if (new_arr != *arr)
            *arr = new_arr;
    } CATCH {
        REPORT_C_ERR("Couldn't reallocate array");
        return FALSE;
    }
    END_TRY;
    return TRUE;
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
        void *alloc = malloc(size);
        if (!alloc)
            return ALLOC_ERR;
        memset(alloc, 0, size);
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

