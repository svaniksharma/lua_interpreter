#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(cond, msg, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "[%s:%s:%d] " msg "\n", \
                    __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0);

#endif
