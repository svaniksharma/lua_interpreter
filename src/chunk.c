#include "chunk.h"
#include "debug.h"
#include "structs.h"
#include <string.h>

ERR init_chunk(LUA_CHUNK *chunk) {
    chunk->scope = 0;
    CHECK(init_dyn_arr(&chunk->code, sizeof(uint8_t)) != FAIL);
    CHECK(init_dyn_arr(&chunk->values, sizeof(LUA_OBJ)) != FAIL);
    return SUCCESS;
lua_err:
    destroy_chunk(chunk);
    return FAIL;
}

LUA_BOOL write_const_chunk(LUA_CHUNK *chunk, LUA_OBJ *o) {
    return ADD_DYN_ARR(&chunk->values, o);
}

LUA_BOOL write_byte_chunk(LUA_CHUNK *chunk, uint8_t byte) {
    return ADD_DYN_ARR(&chunk->code, &byte);
}

void destroy_chunk(LUA_CHUNK *chunk) {
    chunk->scope = 0;
    destroy_dyn_arr(&chunk->code);
    destroy_dyn_arr(&chunk->values);
}
