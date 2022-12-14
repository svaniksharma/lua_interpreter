#ifndef COMPILE_H
#define COMPILE_H

#include "lexer.h"
#include "chunk.h"
#include "lua_string.h"
#include "vm.h"

#define MAX_ERR_BUF 100

typedef enum lua_prec {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_COMPARISON,
    PREC_CAT,
    PREC_TERM,
    PREC_FACTOR,
    PREC_EXP,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY,
} LUA_PREC;

#define CHECK_TYPE(p, t) (p->curr.type == t)

typedef struct lua_parser {
    SRCBUF *buf;
    TOKEN prev;
    TOKEN curr;
    LUA_BOOL panic;
    LUA_BOOL had_err;
} LUA_PARSER;

LUA_PARSER init_lua_parser(SRCBUF *buf);

typedef void (*parse_rule_fn)(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);

typedef struct lua_parse_rule {
    parse_rule_fn prefix;
    parse_rule_fn infix;
    LUA_PREC precedence;
} LUA_PARSE_RULE;

void run(LUA_VM *vm, char *source, int length);

#endif
