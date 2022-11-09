#ifndef LEX_H
#define LEX_H

#define SINGULAR_TOKEN(buf, token_type) \
{ \
    buf->index++; \
    return init_token(token_type, NULL, 0); \
}

#define LOOKAHEAD(buf, c, token_success, token_failure) \
{ \
    TOKEN t = buf->src[buf->index + 1] == c ? init_token(token_success, NULL, 0) \
                                            : init_token(token_failure, NULL, 0); \
    buf->index = buf->index + ((t.type == token_success) ? 2 : 1); \
    return t; \
}

#define MATCH_TO_END(buf, str, len, token_type) \
{ \
    ++buf->index; \
    if (match_to_end(buf, str, len)) { \
        return init_token(token_type, NULL, 0); \
    } \
    break; \
}

#define MATCH_BRANCH(buf, match_func) \
{ \
    ++buf->index; \
    TOKEN_TYPE t = match_func(buf); \
    if (t != TOKEN_ID) \
        return init_token(t, NULL, 0); \
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

#endif
