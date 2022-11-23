#include "error.h"
#include "debug.h"
#include "structs.h"
#include "compile.h"

#define MAX_LINE_LEN 1000

static void lua_repl() {
    LUA_VM vm = { 0 };
    init_vm(&vm);
    char line[MAX_LINE_LEN+1] = { 0 };
    printf("> ");
    while (fgets(line, MAX_LINE_LEN+1, stdin) != NULL) {
        if (strlen(line) > 0 && line[0] != '\n')
            run(&vm, line, strlen(line));
        printf("> ");
    }
    if (errno) {
        REPORT_LUA_ERR("fgets: %s", strerror(errno));
        errno = 0;
    }
    destroy_vm(&vm);
}

static char *read_file(char *source, int *length) {
    char *buf = NULL;
    FILE *fp = NULL;
    fp = fopen(source, "r");
    CHECK(fp != NULL);
    CHECK(fseek(fp, 0, SEEK_END) >= 0);
    int size = ftell(fp);
    CHECK(size >= 0);
    *length = size;
    CHECK(fseek(fp, 0, SEEK_SET) >= 0);
    CHECK(SAFE_ALLOC(&buf, size+1) != ALLOC_ERR);
    CHECK(fread(buf, 1, size, fp) >= 0);
    buf[size] = '\0';
    CHECK(fclose(fp) == 0);
    return buf;
lua_err:
    SAFE_FREE(&buf);
    if (fp != NULL)
        fclose(fp);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        lua_repl();
    } else {
        int length = 0;
        char *source = read_file(argv[1], &length);
        if (source == NULL)
            return 1;
        LOG_DEBUG("Running %s", argv[1]);
        LUA_VM vm = { 0 };
        init_vm(&vm);
        run(&vm, source, length);
        destroy_vm(&vm);
    }
    return 0;
}
