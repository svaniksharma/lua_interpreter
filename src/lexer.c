#include "debug.h"
#include "structs.h"
#include "lexer.h"
#include <stdio.h>

static TOKEN init_token(TOKEN_TYPE type, char *lexeme, int lexeme_len) {
    TOKEN tok = { 0 };
    tok.type = type;
    tok.lexeme = lexeme;
    tok.lexeme_len = lexeme_len;
    return tok;
}

static TOKEN consume_str_literal(SRCBUF *buf, char quote) {
    int i = 0;
    for (i = buf->index; i < buf->length; i++) {
        if (buf->src[i] == quote)
            break;
    }
    TOKEN_TYPE type = (i == buf->length) ? TOKEN_ERR : TOKEN_STR;
    TOKEN tok = init_token(type, buf->src + buf->index, i - 1 - buf->index);
    buf->index =  (i < buf->length) ? i + 1 : buf->length;
    return tok;
}

static LUA_BOOL check_comment(SRCBUF *buf) {
    if (buf->src[buf->index] == '[' && buf->src[buf->index + 1] == '[') {
        buf->index += 2;
        int tokens_counted = 0;
        const int tokens_needed[] = { '-', '-', ']', ']' };
        for (; buf->index < buf->length; buf->index++) {
            char c = buf->src[buf->index];
            if (c == tokens_needed[tokens_counted])
                ++tokens_counted;
            else
                tokens_counted = 0;
            if (tokens_counted == sizeof(tokens_needed) / sizeof(int))
                break;
        }
        return (tokens_counted < sizeof(tokens_needed) / sizeof(int)) ? FALSE : TRUE;
    } else {
        for (; buf->index < buf->length; buf->index++)
            if (buf->src[buf->index] == '\n')
                break;
    }
    return TRUE;
}

static LUA_BOOL is_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static LUA_BOOL is_digit(char c) {
    return c >= '0' && c <= '9';
}

static LUA_BOOL is_valid_id_char(char c) {
    return is_letter(c) || is_digit(c) || c == '_';
}

static LUA_BOOL match_to_end(SRCBUF *buf, const char *str) {
    int i = 0;
    LUA_BOOL matched = TRUE;
    for (i = buf->index; i < buf->length && is_valid_id_char(buf->src[i]); i++) {
        if (buf->src[i] != str[i - buf->index]) {
            matched = FALSE;
        }
    }
    buf->index = i;
    return matched;
}

// TODO: match a number literal, identifier, reserved words
static TOKEN consume_remaining(SRCBUF *buf) {
    if (is_letter(buf->src[buf->index]) || buf->src[buf->index] == '_') {
        TOKEN_TYPE t = TOKEN_ID;
        switch (buf->src[buf->index]) {
            case 'a': MATCH_TO_END(buf, "nd", TOKEN_AND);
            case 'e': MATCH_TO_END(buf, "nd", TOKEN_END);
            case 'w': MATCH_TO_END(buf, "hile", TOKEN_WHILE);
            case 'b': MATCH_TO_END(buf, "reak", TOKEN_BREAK);
            case 'l': MATCH_TO_END(buf, "ocal", TOKEN_LOCAL);
            case 'd': MATCH_TO_END(buf, "o", TOKEN_DO);
            case 'o': MATCH_TO_END(buf, "r", TOKEN_OR);
            case 'u': MATCH_TO_END(buf, "ntil", TOKEN_UNTIL);
            case 'i': {
                if (buf->src[buf->index + 1] == 'f')
                    t = TOKEN_IF;
                else if (buf->src[buf->index + 1] == 'n')
                    t = TOKEN_IN;
                buf->index += 2;
            }
            case 'r': {
            }
            case 'f': {
            }
            case 't': {
            }
            case 'n': {
            }
        }
        return init_token(t, NULL, 0);
    }
}

TOKEN scan_next_token(SRCBUF *buf) {
    while (buf->index < buf->length) {
        char c = buf->src[buf->index];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            ++buf->index;
        else if (c == '-' && buf->src[buf->index + 1] == '-') {
            buf->index += 2;
            if (!check_comment(buf))
                return init_token(TOKEN_ERR, NULL, 0);
        } else if (c == '-') {
            ++buf->index;
            return init_token(TOKEN_MINUS, NULL, 0);
        } else
            break;
    }
    if (buf->index == buf->length)
        return init_token(TOKEN_EOF, NULL, 0);
    switch (buf->src[buf->index]) {
        case '\0': SINGULAR_TOKEN(buf, TOKEN_EOF);
        case '+': SINGULAR_TOKEN(buf, TOKEN_ADD);
        case '*': SINGULAR_TOKEN(buf, TOKEN_MULT);
        case '/': SINGULAR_TOKEN(buf, TOKEN_DIV);
        case '^': SINGULAR_TOKEN(buf, TOKEN_EXP);
        case '(': SINGULAR_TOKEN(buf, TOKEN_LEFT_PAREN);
        case ')': SINGULAR_TOKEN(buf, TOKEN_RIGHT_PAREN);
        case '{': SINGULAR_TOKEN(buf, TOKEN_LEFT_BRACE);
        case '}': SINGULAR_TOKEN(buf, TOKEN_RIGHT_BRACE);
        case '[': SINGULAR_TOKEN(buf, TOKEN_LEFT_BRACKET);
        case ']': SINGULAR_TOKEN(buf, TOKEN_RIGHT_BRACKET);
        case ':': SINGULAR_TOKEN(buf, TOKEN_COLON);
        case ';': SINGULAR_TOKEN(buf, TOKEN_SEMICOLON);
        case '"':
        case '\'':
            ++buf->index;
            return consume_str_literal(buf, buf->src[buf->index - 1]);
        case '<': LOOKAHEAD(buf, '=', TOKEN_LE, TOKEN_LT);
        case '>': LOOKAHEAD(buf, '=', TOKEN_GE, TOKEN_GT);
        case '=': LOOKAHEAD(buf, '=', TOKEN_EQ, TOKEN_ASSIGN);
        case '~': LOOKAHEAD(buf, '=', TOKEN_NE, TOKEN_ERR);
        case '.': LOOKAHEAD(buf, '.', TOKEN_CAT, TOKEN_DOT);
        default:
            break;
    }
    TOKEN t = consume_remaining(buf);
    return t;
}
