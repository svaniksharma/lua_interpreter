#include "debug.h"
#include "lexer.h"
#include "vm.h"
#include "compile.h"
#include "table.h"
#include <stdio.h>

static void report_parse_err(LUA_PARSER *p, TOKEN *token, const char *msg) {
    if (p->panic)
        return;
    p->panic = TRUE;
    char errbuf[MAX_ERR_BUF+1] = { 0 };
    strncpy(errbuf, token->lexeme, token->lexeme_len);
    fprintf(stderr, "lua: %s: found \"%s\"\n", msg, errbuf);
    p->had_err = TRUE;
}

static void advance_parser(LUA_PARSER *p) {
    p->prev = p->curr;
    p->curr = scan_next_token(p->buf);
    if (p->curr.type == TOKEN_ERR) {
        report_parse_err(p, &p->curr, "Could not parse token");
        return;
    }
}

static void parse_expr(LUA_CHUNK *c, LUA_PARSER *p);
static void parse_prec(LUA_CHUNK *c, LUA_PARSER *p, LUA_PREC prec);
static void grouping(LUA_CHUNK *c, LUA_PARSER *p);
static void number(LUA_CHUNK *c, LUA_PARSER *p);
// static void string(LUA_CHUNK *c, LUA_PARSER *p);
static void _true(LUA_CHUNK *c, LUA_PARSER *p);
static void _false(LUA_CHUNK *c, LUA_PARSER *p);
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
    [TOKEN_EXP]           = {NULL,     binary, PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DIV]           = {NULL,     binary, PREC_FACTOR},
    [TOKEN_MULT]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_EQ]            = {NULL,     binary, PREC_EQUALITY},
    [TOKEN_ASSIGN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GT]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_GE]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LT]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LE]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_ID]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_STR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NUM]           = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     binary, PREC_AND},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {_false,   NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NOT]           = {unary,    NULL,   PREC_TERM},
    [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     binary, PREC_OR},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {_true,    NULL,   PREC_NONE},
    [TOKEN_LOCAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ERR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};


static void grouping(LUA_CHUNK *c, LUA_PARSER *p) {
    parse_expr(c, p);
    if (p->curr.type != TOKEN_RIGHT_PAREN) {
        report_parse_err(p, &p->curr, "Expected ')' after expression");
        return;
    }
    advance_parser(p);
}

static void number(LUA_CHUNK *c, LUA_PARSER *p) {
    LUA_REAL r = strtod(p->prev.lexeme, NULL);
    LUA_OBJ o = init_lua_obj(REAL, &r);
    write_const_chunk(c, &o);
    write_byte_chunk(c, OP_CONST);
    write_byte_chunk(c, SIZE_DYN_ARR(c->values)-1);
}

static void _true(LUA_CHUNK *c, LUA_PARSER *p) {
    LUA_BOOL b = TRUE;
    LUA_OBJ o = init_lua_obj(BOOL, &b);
    write_const_chunk(c, &o);
    write_byte_chunk(c, OP_CONST);
    write_byte_chunk(c, SIZE_DYN_ARR(c->values)-1);
}

static void _false(LUA_CHUNK *c, LUA_PARSER *p) {
    LUA_BOOL b = FALSE;
    LUA_OBJ o = init_lua_obj(BOOL, &b);
    write_const_chunk(c, &o);
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
            write_byte_chunk(c, OP_NOT);
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
        case TOKEN_EXP:
            write_byte_chunk(c, OP_EXP);
            break;
        case TOKEN_EQ:
            write_byte_chunk(c, OP_EQ);
            break;
        case TOKEN_LE:
            write_byte_chunk(c, OP_LE);
            break;
        case TOKEN_LT:
            write_byte_chunk(c, OP_LT);
            break;
        case TOKEN_GE:
            write_byte_chunk(c, OP_GE);
            break;
        case TOKEN_GT:
            write_byte_chunk(c, OP_GT);
            break;
        case TOKEN_AND:
            write_byte_chunk(c, OP_AND);
            break;
        case TOKEN_OR:
            write_byte_chunk(c, OP_OR);
            break;
        default:
            return;
    }
}

static void parse_prec(LUA_CHUNK *c, LUA_PARSER *p, LUA_PREC prec) {
    advance_parser(p);
    prefix_fn prefix_rule = parse_rules[p->prev.type].prefix;
    if (prefix_rule == NULL) {
        report_parse_err(p, &p->prev, "Expected expression");
        return;
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

LUA_PARSER init_lua_parser(SRCBUF *buf) {
    LUA_PARSER p = { 0 };
    p.buf = buf;
    p.had_err = FALSE;
    p.panic = FALSE;
    return p;
}

LUA_BOOL compile(LUA_CHUNK *c, char *source, int length) {
    SRCBUF buf = init_src_buf(source, length);
    LUA_PARSER p = init_lua_parser(&buf);
    advance_parser(&p);
    parse_expr(c, &p);
    if (p.curr.type != TOKEN_EOF)
        report_parse_err(&p, &p.curr, "Unexpected token");
    return p.had_err == FALSE;
}

void run(char *source, int length) {
    LUA_CHUNK chunk = init_chunk();
    if (!compile(&chunk, source, length))
        return;
    write_byte_chunk(&chunk, OP_RETURN);
    LUA_VM vm = { 0 };
    init_vm(&vm);
    run_vm(&vm, &chunk);
    destroy_chunk(&chunk);
    destroy_vm(&vm);
}
