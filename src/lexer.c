#include "debug.h"
#include "structs.h"
#include "lexer.h"
#include <stdio.h>

static LUA_BOOL is_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static LUA_BOOL is_digit(char c) {
    return c >= '0' && c <= '9';
}

static LUA_BOOL is_valid_id_char(char c) {
    return is_letter(c) || is_digit(c) || c == '_';
}

static TOKEN init_token(TOKEN_TYPE type, char *lexeme, int lexeme_len) {
    TOKEN tok = { 0 };
    tok.type = type;
    tok.lexeme = lexeme;
    tok.lexeme_len = lexeme_len;
    return tok;
}

static TOKEN consume_str_literal(SRCBUF *buf) {
    char quote = CURR(buf);
    ADVANCE(buf);
    int i = 0;
    for (i = buf->index; i < buf->length; i++) {
        if (buf->src[i] == quote)
            break;
    }
    TOKEN_TYPE type = (i == buf->length) ? TOKEN_ERR : TOKEN_STR;
    TOKEN tok = init_token(type, buf->src + buf->index, i - buf->index);
    buf->index =  (i < buf->length) ? i + 1 : buf->length;
    return tok;
}

static LUA_BOOL is_exponent(SRCBUF *buf, LUA_BOOL exp_seen) {
    return !exp_seen && (CURR(buf) == 'E' || CURR(buf) == 'e') 
        && (LOOKAHEAD(buf) == '+' || LOOKAHEAD(buf) == '-' || is_digit(LOOKAHEAD(buf)));
}

static LUA_BOOL is_frac(SRCBUF *buf, LUA_BOOL frac_seen) {
    return !frac_seen && CURR(buf) == '.';
}

static TOKEN consume_num_literal(SRCBUF *buf) {
    int start = buf->index;
    LUA_BOOL frac_seen = FALSE;
    LUA_BOOL exp_seen = FALSE;
    while (CAN_ADVANCE(buf)) {
        if (!is_digit(CURR(buf)) && !is_exponent(buf, exp_seen) && !is_frac(buf, frac_seen)) {
            break;
        } else if (is_exponent(buf, exp_seen)) {
            DOUBLE_ADVANCE(buf);
            exp_seen = TRUE;
            frac_seen = (frac_seen) ? FALSE : frac_seen;
        } else if (is_frac(buf, frac_seen)) {
            frac_seen = TRUE;
        }
        ADVANCE(buf);
    }
    return init_token(TOKEN_NUM, buf->src + start, buf->index - start);
}

static LUA_BOOL check_comment(SRCBUF *buf) {
    if (CURR(buf) == '[' && LOOKAHEAD(buf) == '[') {
        DOUBLE_ADVANCE(buf);
        int tokens_counted = 0;
        const int tokens_needed[] = { '-', '-', ']', ']' };
        while (CAN_ADVANCE(buf)) {
            char c = CURR(buf);
            ADVANCE(buf);
            if (c == tokens_needed[tokens_counted])
                ++tokens_counted;
            else
                tokens_counted = 0;
            if (tokens_counted == sizeof(tokens_needed) / sizeof(int))
                break;
        }
        return (tokens_counted < sizeof(tokens_needed) / sizeof(int)) ? FALSE : TRUE;
    } else {
        while (CAN_ADVANCE(buf)) {
            if (CURR(buf) == '\n')
                break;
            ADVANCE(buf);
        }
    }
    return TRUE;
}

static LUA_BOOL match_to_end(SRCBUF *buf, const char *str, int len) {
   int k = 0;
   while (k < len && CAN_ADVANCE(buf) && is_valid_id_char(CURR(buf))) {
       if (CURR(buf) != str[k])
           return FALSE;
       ADVANCE(buf);
       ++k;
   }
   return k == len && !is_valid_id_char(CURR(buf));
}

static void consume_id(SRCBUF *buf) {
    while (CAN_ADVANCE(buf) && is_valid_id_char(CURR(buf)))
        ADVANCE(buf);
}

static TOKEN_TYPE match_e(SRCBUF *buf) {
    switch (CURR(buf)) {
        case 'n': MATCH_RET_TYPE(buf, "d", 1, TOKEN_END);
        case 'l':
            ADVANCE(buf);
            if (match_to_end(buf, "se", 2))
                return TOKEN_ELSE;
            else if (match_to_end(buf, "if", 2))
                return TOKEN_ELSEIF;
            break;
    }
    return TOKEN_ID;
}

static TOKEN_TYPE match_i(SRCBUF *buf) {
   if (CURR(buf) == 'f' && !is_valid_id_char(LOOKAHEAD(buf))) {
       ADVANCE(buf);
       return TOKEN_IF;
   } else if (CURR(buf) == 'n' && !is_valid_id_char(LOOKAHEAD(buf))) {
       ADVANCE(buf);
       return TOKEN_IN;
   }
   return TOKEN_ID;
}

static TOKEN_TYPE match_r(SRCBUF *buf) {
    if (CURR(buf) == 'e') {
        ADVANCE(buf);
        switch (CURR(buf)) {
            case 'p': MATCH_RET_TYPE(buf, "eat", 3, TOKEN_REPEAT);
            case 't': MATCH_RET_TYPE(buf, "urn", 3, TOKEN_RETURN);
        }
    }
    return TOKEN_ID;
}

static TOKEN_TYPE match_f(SRCBUF *buf) {
    switch (CURR(buf)) {
        case 'o': MATCH_RET_TYPE(buf, "r", 1, TOKEN_FOR);
        case 'a': MATCH_RET_TYPE(buf, "lse", 3, TOKEN_FALSE);
        case 'u': MATCH_RET_TYPE(buf, "nction", 6, TOKEN_FUNCTION);
    }
    return TOKEN_ID;
}

static TOKEN_TYPE match_t(SRCBUF *buf) {
    switch (CURR(buf)) {
        case 'r': MATCH_RET_TYPE(buf, "ue", 2, TOKEN_TRUE);
        case 'h': MATCH_RET_TYPE(buf, "en", 2, TOKEN_THEN);
    }
    return TOKEN_ID;
}

static TOKEN_TYPE match_n(SRCBUF *buf) {
    switch (CURR(buf)) {
        case 'i': MATCH_RET_TYPE(buf, "l", 1, TOKEN_NIL);
        case 'o': MATCH_RET_TYPE(buf, "t", 1, TOKEN_NOT);
    }
    return TOKEN_ID;
}

static TOKEN consume_remaining(SRCBUF *buf) {
    if (is_letter(CURR(buf)) || CURR(buf) == '_') {
        int start = buf->index;
        switch (CURR(buf)) {
            case 'a': MATCH_TO_END(buf, "nd", 2, TOKEN_AND);
            case 'w': MATCH_TO_END(buf, "hile", 4, TOKEN_WHILE);
            case 'b': MATCH_TO_END(buf, "reak", 4, TOKEN_BREAK);
            case 'l': MATCH_TO_END(buf, "ocal", 4, TOKEN_LOCAL);
            case 'd': MATCH_TO_END(buf, "o", 1, TOKEN_DO);
            case 'o': MATCH_TO_END(buf, "r", 1, TOKEN_OR);
            case 'u': MATCH_TO_END(buf, "ntil", 4, TOKEN_UNTIL);
            case 'e': MATCH_BRANCH(buf, match_e);
            case 'i': MATCH_BRANCH(buf, match_i);
            case 'r': MATCH_BRANCH(buf, match_r);
            case 'f': MATCH_BRANCH(buf, match_f);
            case 't': MATCH_BRANCH(buf, match_t);
            case 'n': MATCH_BRANCH(buf, match_n);
        }
        consume_id(buf);
        return init_token(TOKEN_ID, buf->src + start, buf->index - start);
    } else if (is_digit(CURR(buf))) {
        return consume_num_literal(buf);
    }
    return init_token(TOKEN_ERR, NULL, 0);
}

SRCBUF init_src_buf(char *src, int length) {
    SRCBUF buf = { 0 };
    buf.src = src;
    buf.length = length;
    buf.index = 0;
    return buf;
}

TOKEN scan_next_token(SRCBUF *buf) {
    while (CAN_ADVANCE(buf)) {
        char c = CURR(buf);
        if (c == ' ' || c == '\t' || c == '\r') {
            ADVANCE(buf);
        } else if (c == '-' && LOOKAHEAD(buf) == '-') {
            DOUBLE_ADVANCE(buf);
            if (!check_comment(buf))
                return init_token(TOKEN_ERR, NULL, 0);
        } else if (c == '-') {
            ADVANCE(buf);
            return init_token(TOKEN_MINUS, NULL, 0);
        } else
            break;
    }
    switch (CURR(buf)) {
        case '\0': SINGULAR_TOKEN(buf, TOKEN_EOF);
        case '\n': SINGULAR_TOKEN(buf, TOKEN_LINE_BREAK);
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
        case ',': SINGULAR_TOKEN(buf, TOKEN_COMMA);
        case '"':
        case '\'':
            return consume_str_literal(buf);
        case '<': LOOKAHEAD_BRANCH(buf, '=', TOKEN_LE, TOKEN_LT);
        case '>': LOOKAHEAD_BRANCH(buf, '=', TOKEN_GE, TOKEN_GT);
        case '=': LOOKAHEAD_BRANCH(buf, '=', TOKEN_EQ, TOKEN_ASSIGN);
        case '~': LOOKAHEAD_BRANCH(buf, '=', TOKEN_NE, TOKEN_ERR);
        case '.': LOOKAHEAD_BRANCH(buf, '.', TOKEN_CAT, TOKEN_DOT);
        default:
            break;
    }
    TOKEN t = consume_remaining(buf);
    return t;
}
