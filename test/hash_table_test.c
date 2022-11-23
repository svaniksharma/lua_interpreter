#include "../src/lua_string.h"
#include "../src/table.h"
#include "../src/structs.h"
#include "../src/debug.h"

#define NUM_STRS 5

static void do_hash_test(TABLE *t, LUA_STR *str) {
    LUA_REAL v = 4;
    LUA_OBJ obj = init_lua_obj(REAL, &v);
    /* Check table doesn't contain string */
    ASSERT(get_table(t, str) == NULL, "Table contains string");
    /* Insert the string, and check it's there */
    int old_size = size_table(t);
    put_table(t, str, &obj);
    LUA_OBJ *value = get_table(t, str);
    ASSERT(value != NULL && IS_NUM(*value) && AS_NUM(*value) == 4, "Value not inserted");
    ASSERT(size_table(t) == old_size + 1, "Incorrect size: %d", size_table(t));
    /* Insert a copy of the string, and check corresponding changes */
    LUA_STR *cpy = init_lua_str(str->str, str->size);
    v = 90;
    old_size = size_table(t);
    obj = init_lua_obj(REAL, &v);
    put_table(t, cpy, &obj);
    value = get_table(t, cpy);
    ASSERT(value != NULL && IS_NUM(*value) && AS_NUM(*value) == 90, "Value not updated");
    ASSERT(size_table(t) == old_size, "Incorrect size: %d", size_table(t));
    destroy_lua_str(&cpy);
}

int main(int argc, char *argv[]) {
    TABLE t = { 0 };
    init_table(&t, str_obj_hash, equals_str);
    char *strs[NUM_STRS] = {"test", "_a_var_name", "number1234", "key", "json_data"};
    for (int i = 0; i < NUM_STRS; i++) {
        LUA_STR *str = init_lua_str(strs[i], strlen(strs[i]));
        do_hash_test(&t, str);
        destroy_lua_str(&str);
    }
    destroy_table(&t);
    printf("Tests passed\n");
    return 0;
}
