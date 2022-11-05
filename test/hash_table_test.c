#include "../src/structs.h"
#include "../src/debug.h"
#include <stdlib.h>

#define N 100000
#define NUM_KEYS 100000
#define PUT_OP 0
#define REMOVE_OP 1
#define MAX_STR_LEN 30

#define MIN_ASCII 97
#define MAX_ASCII 122

static void gen_rand_str(char **str_ptr) {
    int size = arc4random_uniform(MAX_STR_LEN) + 1;
    SAFE_ALLOC(str_ptr, size);
    char *str = *str_ptr;
    for (int i = 0; i < size; i++) {
        unsigned int c = arc4random_uniform(MAX_ASCII - MIN_ASCII + 1) + MIN_ASCII;
        str[i] = c;
    }
    str[size] = '\0';
} 

static void rand_ops_test() {
    TABLE t = { 0 };
    init_table(&t, str_hash);
    char *keys[NUM_KEYS] = { 0 };
    char *values[NUM_KEYS] = { 0 };
    for (int i = 0; i < NUM_KEYS; i++) {
        char *key = NULL;
        char *value = NULL;
        gen_rand_str(&key);
        gen_rand_str(&value);
        keys[i] = key;
        values[i] = value;
    }
    for (int i = 0; i < N; i++) {
        int op = arc4random_uniform(2);
        int index = arc4random_uniform(NUM_KEYS);
        printf("%s on pair (%s, %s)\n", 
                op == PUT_OP ? "PUT_OP" : "REMOVE_OP", keys[index], values[index]);
        if (op == PUT_OP) {
            int old_size = size_table(&t);
            int old_capacity = t.capacity; // bit of a hack, but just for testing (see below)
            LUA_BOOL inserted = put_table(&t, keys[index], values[index]);
            char *got = get_table(&t, keys[index]);
            ASSERT(got == values[index], "Expected %s got %s", values[index], got);
            int new_size = size_table(&t);
            int expected_size = inserted ? old_size + 1 : old_size;
            /* If capacity changed, then size different, so don't check */
            if (old_capacity == t.capacity)
                ASSERT(new_size == expected_size, "Expected %d got %d", expected_size, new_size);
        } else { // REMOVE_OP
            int old_size = size_table(&t);
            char *got = get_table(&t, keys[index]);
            LUA_BOOL removed = remove_table(&t, keys[index]);
            if (!removed)
                ASSERT(got == NULL, "Before removal, get_table() returned non-NULL value");
            else {
                ASSERT(got != NULL, "Before removal, get_table() returned a NULL value");
                got = get_table(&t, keys[index]);
                ASSERT(got == NULL, "Expected NULL after remove, but get_table() return non-NULL");
            }
            ASSERT(size_table(&t) == old_size, "Expected size to stay same, but changed");
        }
    }
    destroy_table(&t);
    for (int i = 0; i < NUM_KEYS; i++) {
        SAFE_FREE(&keys[i]);
        SAFE_FREE(&values[i]);
    }
}

int main(int argc, char *argv[]) {
    char *key = "a_random_key";
    char *value = "a_random_value";
    TABLE t = { 0 };
    init_table(&t, str_hash);
    put_table(&t, key, value);
    char *got = get_table(&t, key);
    ASSERT(value == got, "Expected %s got %s\n", value, got);
    ASSERT(size_table(&t) == 1, "Expected %d got %d\n", 1, size_table(&t));
    remove_table(&t, key);
    got = get_table(&t, key);
    ASSERT(got == NULL, "Expected null got %s\n", got);
    ASSERT(size_table(&t) == 1, "Expected %d got %d\n", 1, size_table(&t));
    destroy_table(&t);
    printf("Random Operations Test...\n");
    rand_ops_test();
    return 0;
}
