#ifndef CHUNK_H
#define CHUNK_H

#include "structs.h"

typedef enum opcode {
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
    OP_EXP,
    OP_EQ,
    OP_NE,
    OP_GT,
    OP_GE,
    OP_LT,
    OP_LE,
    OP_CAT,
    OP_AND,
    OP_OR,
    OP_TRUE,
    OP_FALSE,
    OP_DEF_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_POP,
    OP_CONST,
    OP_NEGATE,
    OP_NOT,
    OP_NIL,
    OP_RETURN,
} LUA_OPCODE;

#ifdef LUA_DEBUG

static char *opcode_str_debug_table[] = {
    [OP_ADD]        = "OP_ADD",
    [OP_SUB]        = "OP_SUB",
    [OP_MULT]       = "OP_MULT",
    [OP_DIV]        = "OP_DIV",
    [OP_EXP]        = "OP_EXP",
    [OP_EQ]         = "OP_EQ",
    [OP_NE]         = "OP_NE",
    [OP_GT]         = "OP_GT",
    [OP_GE]         = "OP_GE",
    [OP_LT]         = "OP_LT",
    [OP_LE]         = "OP_LE",
    [OP_CAT]        = "OP_CAT",
    [OP_AND]        = "OP_AND",
    [OP_OR]         = "OP_OR",
    [OP_TRUE]       = "OP_TRUE",
    [OP_FALSE]      = "OP_FALSE",
    [OP_DEF_GLOBAL] = "OP_DEF_GLOBAL",
    [OP_GET_GLOBAL] = "OP_GET_GLOBAL",
    [OP_SET_GLOBAL] = "OP_SET_GLOBAL",
    [OP_POP]        = "OP_POP",
    [OP_CONST]      = "OP_CONST",
    [OP_NEGATE]     = "OP_NEGATE",
    [OP_NOT]        = "OP_NOT",
    [OP_NIL]        = "OP_NIL",
    [OP_RETURN]     = "OP_RETURN"
};

#endif


#define INITIAL_CAP 10

typedef struct lua_chunk {
    DYN_ARR code;
    DYN_ARR values;
} LUA_CHUNK;

LUA_CHUNK init_chunk();
LUA_BOOL write_const_chunk(LUA_CHUNK *chunk, LUA_OBJ *o);
LUA_BOOL write_byte_chunk(LUA_CHUNK *chunk, uint8_t byte);
void destroy_chunk(LUA_CHUNK *chunk);

#endif
