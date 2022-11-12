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

#define REPORT_C_ERR(msg, ...) \
    do { \
        if (errno) \
            fprintf(stderr, msg ": %s\n", ##__VA_ARGS__, strerror(errno)); \
        else \
            fprintf(stderr, msg "\n", ##__VA_ARGS__); \
    } while (0)

#define REPORT_LUA_ERR(msg, ...) \
    do { \
        fprintf(stderr, msg, ##__VA_ARGS__); \
    } while (0)

#define TRY do { jmp_buf _ex; if (!setjmp(_ex)) {
#define CATCH } else {
#define END_TRY } } while (0)
#define THROW(cond) do { if ((cond)) { longjmp(_ex, FAIL); } } while(0)

#endif
