#ifndef STRUCTS_H
#define STRUCTS_H

#include "error.h"
#include <stdint.h>

#define FALSE 0
#define TRUE 1

ERR safe_alloc(uint8_t **ptr, int size);
void safe_free(uint8_t **ptr);

#endif
