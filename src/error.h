#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>

typedef enum err {
    SUCCESS,
    FAIL,
    ALLOC_ERR,
    FREE_ERR,
} ERR;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            if (errno) \
                fprintf(stderr, "[%s:%s:%d] %s\n", \
                    __FILE__, __FUNCTION__, __LINE__, strerror(errno)); \
            errno = 0; \
            goto lua_err; \
        } \
    } while (0)


#endif
