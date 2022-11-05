#ifndef STRUCTS_H
#define STRUCTS_H

#include "error.h"
#include <stdint.h>

#define FALSE 0
#define TRUE 1

ERR safe_alloc(void **ptr, int size);
void safe_free(void **ptr);

/* So we don't have to explicitly cast to (void **) */
#define SAFE_ALLOC(ptr, size) safe_alloc((void **) (ptr), (size))
#define SAFE_FREE(ptr) safe_free((void **) (ptr))

#endif
