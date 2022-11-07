#include "debug.h"
#include "compile.h"
#include "lexer.h"
#include <stdio.h>

void run(char *source, int length) {
    SRCBUF buf = { 0 };
    buf.src = source;
    buf.length = length;
    buf.index = 0;
    while (buf.index < buf.length) {
        TOKEN t = scan_next_token(&buf);
        LOG_DEBUG("%d", t.type);
        if (t.type == TOKEN_ERR) {
            LOG_DEBUG("FOUND ERROR TOKEN");
            return;
        }
    }
}
