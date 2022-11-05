#include "structs.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

ERR safe_alloc(uint8_t **ptr, int size) {
    if (size <= 0) {
        *ptr = NULL;
        return SUCCESS;
    }
    if (*ptr == NULL) {
        uint8_t *alloc = malloc(size);
        if (!alloc)
            return ALLOC_ERR;
        memset(alloc, 0, size);
        *ptr = alloc;
        return SUCCESS;
    }
    return ALLOC_ERR;
}

void safe_free(uint8_t **ptr) {
    if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
    }
}
