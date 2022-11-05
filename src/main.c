#include "error.h"
#include "structs.h"
#include "compile.h"
#include <stdio.h>


#define MAX_LINE_LEN 1000

static void lua_repl() {
    char line[MAX_LINE_LEN+1];
    printf("> ");
    while (fgets(line, MAX_LINE_LEN+1, stdin) != NULL) {
        run(line); // run the line
        printf("> ");
    }
}

static char *read_file(char *source) {
    char *buf = NULL;
    FILE *fp = NULL;
    TRY {
        fp = fopen(source, "r");
        THROW(fp);
        THROW(fseek(fp, 0, SEEK_END) < 0);
        int size = ftell(fp);
        THROW(size < 0);
        THROW(fseek(fp, 0, SEEK_SET) < 0);
        THROW(SAFE_ALLOC(&buf, size+1));
        THROW(fread(buf, 1, size, fp) < 0);
        buf[size-1] = '\0';
        THROW(fclose(fp) < 0);
        return buf;
    } CATCH {
        REPORT_C_ERR("An error occurred reading the file");
        TRY {
            if (fp != NULL)
                fclose(fp);
        } CATCH {
            REPORT_C_ERR("An error occurred closing the file");
        }
        END_TRY;
        SAFE_FREE(&buf);
    }
    END_TRY;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        lua_repl();
    } else {
        char *source = read_file(argv[1]);
        if (source == NULL)
            return 1;
        run(source);  // run the line
    }
    return 0;
}
