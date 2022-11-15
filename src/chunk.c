#include "chunk.h"
#include "debug.h"
#include "structs.h"

LUA_CHUNK init_chunk() {
    LUA_CHUNK chunk = { 0 };
    init_dyn_arr(&chunk.code, sizeof(uint8_t));
    init_dyn_arr(&chunk.values, sizeof(LUA_OBJ));
    return chunk;
}

LUA_BOOL write_const_chunk(LUA_CHUNK *chunk, LUA_OBJ *o) {
    return ADD_DYN_ARR(&chunk->values, o);
}

LUA_BOOL write_byte_chunk(LUA_CHUNK *chunk, uint8_t byte) {
    return ADD_DYN_ARR(&chunk->code, &byte);
}

void destroy_chunk(LUA_CHUNK *chunk) {
    destroy_dyn_arr(&chunk->code);
    destroy_dyn_arr(&chunk->values);
}
