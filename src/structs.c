#include "structs.h"
#include "error.h"
#include "debug.h"
#include <stdlib.h>
#include <string.h>

/* MEMORY ALLOCATION */

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

