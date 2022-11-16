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
    OP_TRUE,
    OP_FALSE,
    OP_CONST,
    OP_NEGATE,
    OP_NOT,
    OP_AND,
    OP_OR,
    OP_NIL,
    OP_CAT,
    OP_RETURN,
} LUA_OPCODE;


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
