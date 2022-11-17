#include "debug.h"
#include "lexer.h"
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

static void consume_parser(LUA_PARSER *p, TOKEN_TYPE type, const char *msg) {
    if (p->curr.type == type) {
        advance_parser(p);
        return;
    }
    report_parse_err(p, &p->curr, msg);
}

static void sync_err(LUA_PARSER *p) {
    p->panic = FALSE;
    while (!CHECK_TYPE(p, TOKEN_EOF)) {
        switch (p->prev.type) {
            case TOKEN_FUNCTION:
            case TOKEN_LOCAL:
            case TOKEN_FOR:
            case TOKEN_WHILE:
            case TOKEN_IF:
            case TOKEN_RETURN:
                return;
            default:
                ; // continue the loop
        }
        advance_parser(p);
    }
}

static void parse_decl(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm);
static void parse_var_decl(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm);
static void parse_stmt(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL do_advance);
static void parse_expr(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL do_advance);
static void parse_prec(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_PREC prec, LUA_BOOL do_advance);
static void var(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);
static void grouping(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);
static void literal(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);
static void number(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);
static void string(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);
static void unary(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);
static void binary(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);
static void eof(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign);

LUA_PARSE_RULE parse_rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
    [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
    [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_ADD]           = {NULL,     binary, PREC_TERM},
    [TOKEN_EXP]           = {NULL,     binary, PREC_EXP},
    [TOKEN_CAT]           = {NULL,     binary, PREC_CAT},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DIV]           = {NULL,     binary, PREC_FACTOR},
    [TOKEN_MULT]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_EQ]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_NE]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_ASSIGN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GT]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_GE]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LT]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LE]            = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_ID]            = {var,      NULL,   PREC_NONE},
    [TOKEN_STR]           = {string,   NULL,   PREC_NONE},
    [TOKEN_NUM]           = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     binary, PREC_AND},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NOT]           = {unary,    NULL,   PREC_TERM},
    [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     binary, PREC_OR},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
    [TOKEN_LOCAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ERR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {eof,      NULL,   PREC_NONE},
};


static void grouping(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    parse_expr(c, p, vm, TRUE);
    consume_parser(p, TOKEN_RIGHT_PAREN, "Expected ')' after expression");
}

static void number(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    LUA_REAL r = strtod(p->prev.lexeme, NULL);
    LUA_OBJ o = init_lua_obj(REAL, &r);
    write_const_chunk(c, &o);
    write_byte_chunk(c, OP_CONST);
    write_byte_chunk(c, SIZE_DYN_ARR(c->values)-1);
}


static void literal(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    switch (p->prev.type) {
        case TOKEN_FALSE:
            write_byte_chunk(c, OP_FALSE);
            break;
        case TOKEN_TRUE:
            write_byte_chunk(c, OP_TRUE);
            break;
        case TOKEN_NIL:
            write_byte_chunk(c, OP_NIL);
            break;
        default:
            return;
    }
}

static void unary(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    TOKEN_TYPE prev_type = p->prev.type;
    parse_prec(c, p, vm, PREC_UNARY, TRUE);
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

static void binary(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    TOKEN_TYPE type = p->prev.type;
    LUA_PARSE_RULE rule = parse_rules[type];
    parse_prec(c, p, vm, (LUA_PREC) (rule.precedence + 1), TRUE);
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
        case TOKEN_NE:
            write_byte_chunk(c, OP_NE);
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
        case TOKEN_CAT:
            write_byte_chunk(c, OP_CAT);
            break;
        default:
            return;
    }
}

static void string(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    LUA_OBJ obj = { 0 };
    LUA_OBJ *str_obj = get_table_str(&vm->strings, p->prev.lexeme, p->prev.lexeme_len);
    LUA_STR *str = NULL;
    if (IS_NIL(*str_obj)) {
        str = init_lua_str(p->prev.lexeme, p->prev.lexeme_len);
        put_table(&vm->strings, str, NULL);
    } else {
        str = AS_STR(*str_obj);
    }
    obj = init_lua_obj(STR, str);
    write_const_chunk(c, &obj);
    write_byte_chunk(c, OP_CONST);
    write_byte_chunk(c, SIZE_DYN_ARR(c->values)-1);
}

static void eof(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    write_byte_chunk(c, OP_RETURN);
}

static void parse_prec(LUA_CHUNK *c, LUA_PARSER *p, 
        LUA_VM *vm, LUA_PREC prec, LUA_BOOL do_advance) {
    if (do_advance)
        advance_parser(p);
    parse_rule_fn prefix_rule = parse_rules[p->prev.type].prefix;
    if (prefix_rule == NULL) {
        report_parse_err(p, &p->prev, "Expected expression");
        return;
    }
    LUA_BOOL can_assign = prec <= PREC_ASSIGNMENT;
    prefix_rule(c, p, vm, can_assign);
    while (prec <= parse_rules[p->curr.type].precedence) {
        advance_parser(p);
        parse_rule_fn infix_rule = parse_rules[p->prev.type].infix;
        infix_rule(c, p, vm, can_assign);
    }
}

static LUA_BOOL match(LUA_PARSER *p, TOKEN_TYPE t) {
    if (!CHECK_TYPE(p, t))
        return FALSE;
    advance_parser(p);
    return TRUE;
}

static void parse_decl(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm) {
    if (match(p, TOKEN_ID)) {
        LUA_OBJ *name_val = get_table_str(&vm->globals, p->prev.lexeme, p->prev.lexeme_len);
        if (CHECK_TYPE(p, TOKEN_ASSIGN) && IS_NIL(*name_val))
            parse_var_decl(c, p, vm);
        else
            parse_stmt(c, p, vm, FALSE);
    } else
        parse_stmt(c, p, vm, TRUE);
    if (p->panic)
        sync_err(p);
}

static void named_var(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, TOKEN *token, LUA_BOOL can_assign) {
    LUA_STR *str = init_lua_str(token->lexeme, token->lexeme_len);
    CHECK(str != NULL);
    LUA_OBJ obj = init_lua_obj(STR, str);
    write_const_chunk(c, &obj);
    int index = SIZE_DYN_ARR(c->values) - 1;
    if (can_assign && match(p, TOKEN_ASSIGN)) {
        parse_expr(c, p, vm, TRUE);
        write_byte_chunk(c, OP_SET_GLOBAL);
    } else
        write_byte_chunk(c, OP_GET_GLOBAL);
    write_byte_chunk(c, index);
    return;
lua_err:
    return;
}

static void var(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL can_assign) {
    named_var(c, p, vm, &p->prev, can_assign);
}

static int parse_var(LUA_CHUNK *c, LUA_PARSER *p, const char *err_msg) {
    LUA_STR *str = init_lua_str(p->prev.lexeme, p->prev.lexeme_len);
    CHECK(str != NULL);
    LUA_OBJ obj = init_lua_obj(STR, str);
    write_const_chunk(c, &obj);
    advance_parser(p);
    return SIZE_DYN_ARR(c->values)-1;
lua_err:
    return -1;
}

static void define_var(LUA_CHUNK *c, int global_const_index) {
    write_byte_chunk(c, OP_DEF_GLOBAL);
    write_byte_chunk(c, global_const_index);
}

static void parse_var_decl(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm) {
    int global_const_index = parse_var(c, p, "Expected variable name");
    parse_expr(c, p, vm, TRUE);
    consume_parser(p, TOKEN_LINE_BREAK, "Invalid variable declaration");
    define_var(c, global_const_index);
}

static void expr_stmt(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL do_advance) {
    parse_expr(c, p, vm, do_advance);
    consume_parser(p, TOKEN_LINE_BREAK, "Expected end of expression");
    write_byte_chunk(c, OP_POP);
}

static void parse_stmt(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL do_advance) {
    expr_stmt(c, p, vm, do_advance);
}

static void parse_expr(LUA_CHUNK *c, LUA_PARSER *p, LUA_VM *vm, LUA_BOOL do_advance) {
    parse_prec(c, p, vm, PREC_ASSIGNMENT, do_advance);
}

LUA_PARSER init_lua_parser(SRCBUF *buf) {
    LUA_PARSER p = { 0 };
    p.buf = buf;
    p.had_err = FALSE;
    p.panic = FALSE;
    return p;
}

static LUA_BOOL compile(LUA_CHUNK *c, LUA_VM *vm, char *source, int length) {
    SRCBUF buf = init_src_buf(source, length);
    LUA_PARSER p = init_lua_parser(&buf);
    advance_parser(&p);
    while (!match(&p, TOKEN_EOF))
        parse_decl(c, &p, vm);
    return p.had_err == FALSE;
}

void run(LUA_VM *vm, char *source, int length) {
    LUA_CHUNK chunk = init_chunk();
    if (!compile(&chunk, vm, source, length)) {
        destroy_chunk(&chunk);
        return;
    }
    write_byte_chunk(&chunk, OP_RETURN);
    run_vm(vm, &chunk);
    destroy_chunk(&chunk);
}
