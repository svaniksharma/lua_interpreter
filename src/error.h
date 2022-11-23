#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <errno.h>

typedef enum err {
    SUCCESS,
    FAIL,
    ALLOC_ERR,
} ERR;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            if (errno) { \
                fprintf(stderr, "[%s:%s:%d] %s\n", \
                    __FILE__, __FUNCTION__, __LINE__, strerror(errno)); \
                errno = 0; \
            } \
            goto lua_err; \
        } \
    } while (0)

#define REPORT_LUA_ERR(msg, ...) fprintf(stderr, "[%s:%s:%d] " msg "\n", \
                                        __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif
