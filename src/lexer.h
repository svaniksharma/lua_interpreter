#ifndef LEX_H
#define LEX_H

#define CURR(buf) buf->src[buf->index]

#define ADVANCE(buf) \
do {  \
    buf->index = buf->index + ((buf->index < buf->length) ? 1 : 0); \
} while (0)

#define DOUBLE_ADVANCE(buf) \
    ADVANCE(buf);  \
    ADVANCE(buf);

#define LOOKAHEAD(buf) ((buf->index < buf->length) ? buf->src[buf->index + 1] : '\0')
#define CAN_ADVANCE(buf) buf->index < buf->length

#define SINGULAR_TOKEN(buf, token_type) \
{ \
    int start = buf->index; \
    ADVANCE(buf); \
    return init_token(token_type, buf->src + start, 1); \
}

#define LOOKAHEAD_BRANCH(buf, c, token_success, token_failure) \
{ \
    TOKEN t = LOOKAHEAD(buf) == c ? init_token(token_success, buf->src + buf->index, 2) \
                                  : init_token(token_failure, buf->src + buf->index, 1); \
    if (t.type == token_success) { \
        DOUBLE_ADVANCE(buf); \
    } \
    else \
        ADVANCE(buf); \
    return t; \
}

#define MATCH_TO_END(buf, str, len, token_type) \
{ \
    int start = buf->index; \
    ADVANCE(buf); \
    if (match_to_end(buf, str, len)) { \
        return init_token(token_type, buf->src + start, buf->index - start); \
    } \
    break; \
}

#define MATCH_BRANCH(buf, match_func) \
{ \
    int start = buf->index; \
    ADVANCE(buf); \
    TOKEN_TYPE t = match_func(buf); \
    if (t != TOKEN_ID) \
        return init_token(t, buf->src + start, buf->index - start); \
    break; \
}

#define MATCH_RET_TYPE(buf, str, len, token_type) \
{ \
    ADVANCE(buf); \
    if (match_to_end(buf, str, len)) \
        return token_type; \
    break; \
}

typedef enum lua_token_type {
    TOKEN_ADD,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_EXP,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_DOT,
    TOKEN_CAT,
    TOKEN_LE,
    TOKEN_LT,
    TOKEN_GE,
    TOKEN_GT,
    TOKEN_AND,
    TOKEN_END,
    TOKEN_IF,
    TOKEN_ELSEIF,
    TOKEN_ELSE,
    TOKEN_IN,
    TOKEN_ASSIGN,
    TOKEN_STR,
    TOKEN_NUM,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_FUNCTION,
    TOKEN_BREAK,
    TOKEN_LOCAL,
    TOKEN_DO,
    TOKEN_OR,
    TOKEN_THEN,
    TOKEN_COMMA,
    TOKEN_NIL,
    TOKEN_NOT,
    TOKEN_UNTIL,
    TOKEN_REPEAT,
    TOKEN_RETURN,
    TOKEN_ID,
    TOKEN_ERR,
    TOKEN_EOF
} TOKEN_TYPE;

typedef struct lua_token {
    TOKEN_TYPE type;
    int lexeme_len;
    char *lexeme;
} TOKEN;

typedef struct srcbuf {
    int index;
    int length;
    char *src;
} SRCBUF;

TOKEN scan_next_token(SRCBUF *buf);

#ifdef LUA_DEBUG

static char *token_str_debug_table[] = {
    [TOKEN_ADD]             = "TOKEN_ADD",
    [TOKEN_MINUS]           = "TOKEN_MINUS",
    [TOKEN_MULT]            = "TOKEN_MULT",
    [TOKEN_DIV]             = "TOKEN_DIV",
    [TOKEN_EXP]             = "TOKEN_EXP",
    [TOKEN_EQ]              = "TOKEN_EQ",
    [TOKEN_NE]              = "TOKEN_NE",
    [TOKEN_LEFT_PAREN]      = "TOKEN_LEFT_PAREN",
    [TOKEN_RIGHT_PAREN]     = "TOKEN_RIGHT_PAREN",
    [TOKEN_LEFT_BRACE]      = "TOKEN_LEFT_BRACE",
    [TOKEN_RIGHT_BRACE]     = "TOKEN_RIGHT_BRACE",
    [TOKEN_LEFT_BRACKET]    = "TOKEN_LEFT_BRACKET",
    [TOKEN_RIGHT_BRACKET]   = "TOKEN_RIGHT_BRACKET",
    [TOKEN_COLON]           = "TOKEN_COLON",
    [TOKEN_SEMICOLON]       = "TOKEN_SEMICOLON",
    [TOKEN_DOT]             = "TOKEN_DOT",
    [TOKEN_CAT]             = "TOKEN_CAT",
    [TOKEN_LE]              = "TOKEN_LE",
    [TOKEN_LT]              = "TOKEN_LT",
    [TOKEN_GE]              = "TOKEN_GE",
    [TOKEN_GT]              = "TOKEN_GT",
    [TOKEN_AND]             = "TOKEN_AND",
    [TOKEN_END]             = "TOKEN_END",
    [TOKEN_IF]              = "TOKEN_IF",
    [TOKEN_ELSEIF]          = "TOKEN_ELSEIF",
    [TOKEN_ELSE]            = "TOKEN_ELSE",
    [TOKEN_IN]              = "TOKEN_IN",
    [TOKEN_ASSIGN]          = "TOKEN_ASSIGN",
    [TOKEN_STR]             = "TOKEN_STR",
    [TOKEN_NUM]             = "TOKEN_NUM",
    [TOKEN_WHILE]           = "TOKEN_WHILE",
    [TOKEN_FOR]             = "TOKEN_FOR",
    [TOKEN_TRUE]            = "TOKEN_TRUE",
    [TOKEN_FALSE]           = "TOKEN_FALSE",
    [TOKEN_FUNCTION]        = "TOKEN_FUNCTION",
    [TOKEN_BREAK]           = "TOKEN_BREAK",
    [TOKEN_LOCAL]           = "TOKEN_LOCAL",
    [TOKEN_DO]              = "TOKEN_DO",
    [TOKEN_OR]              = "TOKEN_OR",
    [TOKEN_THEN]            = "TOKEN_THEN",
    [TOKEN_COMMA]           = "TOKEN_COMMA",
    [TOKEN_NIL]             = "TOKEN_NIL",
    [TOKEN_NOT]             = "TOKEN_NOT",
    [TOKEN_UNTIL]           = "TOKEN_UNTIL",
    [TOKEN_REPEAT]          = "TOKEN_REPEAT",
    [TOKEN_RETURN]          = "TOKEN_RETURN",
    [TOKEN_ID]              = "TOKEN_ID",
    [TOKEN_ERR]             = "TOKEN_ERR",
    [TOKEN_EOF]             = "TOKEN_EOF",
};

#endif

#endif
