#include "debug.h"
#include "lexer.h"
#include "vm.h"
#include "compile.h"
#include <stdio.h>

static void advance_parser(LUA_PARSER *p) {
    p->prev = p->curr;
    p->curr = scan_next_token(p->buf);
    if (p->curr.type == TOKEN_ERR)
        return; // TODO: handle error
}

static void parse_expr(LUA_CHUNK *c, LUA_PARSER *p);
static void parse_prec(LUA_CHUNK *c, LUA_PARSER *p, LUA_PREC prec);
static void grouping(LUA_CHUNK *c, LUA_PARSER *p);
static void number(LUA_CHUNK *c, LUA_PARSER *p);
static void unary(LUA_CHUNK *c, LUA_PARSER *p);
static void binary(LUA_CHUNK *c, LUA_PARSER *p);

LUA_PARSE_RULE parse_rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
    [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
    [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_ADD]           = {NULL,     binary, PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DIV]           = {NULL,     binary, PREC_FACTOR},
    [TOKEN_MULT]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_EQ]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ASSIGN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GT]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GE]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LT]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LE]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ID]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_STR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NUM]           = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LOCAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ERR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};


static void grouping(LUA_CHUNK *c, LUA_PARSER *p) {
    parse_expr(c, p);
    if (p->curr.type != TOKEN_RIGHT_PAREN)
        REPORT_LUA_ERR("Expected ')' after expression\n");
    advance_parser(p);
}

static void number(LUA_CHUNK *c, LUA_PARSER *p) {
    LUA_REAL r = strtod(p->prev.lexeme, NULL);
    LUA_VAL v = { 0 };
    v.n = r;
    write_const_chunk(c, &v);
    write_byte_chunk(c, OP_CONST);
    write_byte_chunk(c, SIZE_DYN_ARR(c->values)-1);
}

static void unary(LUA_CHUNK *c, LUA_PARSER *p) {
    TOKEN_TYPE prev_type = p->prev.type;
    parse_prec(c, p, PREC_UNARY);
    switch (prev_type) {
        case TOKEN_MINUS:
            write_byte_chunk(c, OP_NEGATE);
            break;
        case TOKEN_NOT:
            break;
        default:
            return;
    }
}

static void binary(LUA_CHUNK *c, LUA_PARSER *p) {
    TOKEN_TYPE type = p->prev.type;
    LUA_PARSE_RULE rule = parse_rules[type];
    parse_prec(c, p, (LUA_PREC) (rule.precedence + 1));
    switch (type) {
        case TOKEN_ADD:
            write_byte_chunk(c, OP_ADD);
            break;
        case TOKEN_MINUS:
            write_byte_chunk(c, OP_SUB);
            break;
        case TOKEN_MULT:
            write_byte_chunk(c, OP_MULT);
            break;
        case TOKEN_DIV:
            write_byte_chunk(c, OP_DIV);
            break;
        default:
            return;
    }
}

static void parse_prec(LUA_CHUNK *c, LUA_PARSER *p, LUA_PREC prec) {
    advance_parser(p);
    prefix_fn prefix_rule = parse_rules[p->prev.type].prefix;
    if (prefix_rule == NULL) {
        return; // TODO: error handling
    }
    prefix_rule(c, p);
    while (prec <= parse_rules[p->curr.type].precedence) {
        advance_parser(p);
        infix_fn infix_rule = parse_rules[p->prev.type].infix;
        infix_rule(c, p);
    }
}

static void parse_expr(LUA_CHUNK *c, LUA_PARSER *p) {
    parse_prec(c, p, PREC_ASSIGNMENT);
}

void run(char *source, int length) {
    SRCBUF buf = { 0 };
    buf.src = source;
    buf.length = length;
    buf.index = 0;
    LUA_PARSER p = { 0 };
    p.buf = &buf;
    advance_parser(&p);
    LUA_CHUNK chunk = { 0 };
    init_chunk(&chunk);
    parse_expr(&chunk, &p);
    write_byte_chunk(&chunk, OP_RETURN);
    LUA_VM vm = { 0 };
    init_vm(&vm);
    run_vm(&vm, &chunk);
    destroy_chunk(&chunk);
    destroy_vm(&vm);
}
