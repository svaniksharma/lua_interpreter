#include "debug.h"
#include "compile.h"
#include "lexer.h"
#include <stdio.h>

void run(char *source, int length) {
    SRCBUF buf = { 0 };
    buf.src = source;
    buf.length = length;
    buf.index = 0;
    TOKEN t = { 0 };
    while (t.type != TOKEN_EOF) {
        t = scan_next_token(&buf);
        if (t.type == TOKEN_ERR) {
            LOG_DEBUG("FOUND ERROR TOKEN");
            return;
        }
        char name[100] = { 0 };
        snprintf(name, t.lexeme_len+1, "%s", t.lexeme);
        LOG_DEBUG("Token Type: %d; Token String: %s", t.type, name);
    }
}
