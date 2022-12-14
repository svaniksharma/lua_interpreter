#include "../src/structs.h"
#include "../src/debug.h"
#include <time.h>

#define N 10

int main(int argc, char *argv[]) {
    srand((unsigned int) time(NULL));
    /* zero-size edge case */
    int alloc_size = 0;
    uint8_t *ptr = NULL;
    int ret = SAFE_ALLOC(&ptr, alloc_size);
    printf("Allocating zero size; alloc err: %s\n", ret == ALLOC_ERR ? "true" : "false");
    ASSERT(ptr == NULL, "Expected ptr to be NULL");
    SAFE_FREE(&ptr);
    ASSERT(ptr == NULL, "Expected ptr to be NULL");
    /* negative-size edge case */
    alloc_size = -rand();
    ret = SAFE_ALLOC(&ptr, alloc_size);
    printf("Allocating negative size: safe_alloc(%d); alloc err: %s\n", 
            alloc_size, ret == ALLOC_ERR ? "true" : "false");
    ASSERT(ptr == NULL, "Expected ptr to be NULL");
    SAFE_FREE(&ptr);
    ASSERT(ptr == NULL, "Expected ptr to be NULL");
    /* Test a couple cases with random values */
    for (int i = 0; i < N; i++) {
        int alloc_size = rand();
        int ret = SAFE_ALLOC((void **) &ptr, alloc_size);
        printf("test %d: safe_alloc(%d); alloc err: %s\n", 
                i + 1, alloc_size, ret == ALLOC_ERR ? "true" : "false");
        SAFE_FREE(&ptr);
        ASSERT(ptr == NULL, "Expected ptr to be NULL");
    }
    printf("Tests passed\n");
    return 0;
}
