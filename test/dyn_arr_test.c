#include "../src/debug.h"
#include "../src/structs.h"

int main(int argc, char *argv[]) {
    const int arr[] = { 8, 4, 1, 9, 10, -202, 4, 90, 100, 23, 86 };
    DYN_ARR d = { 0 };
    ASSERT(init_dyn_arr(&d, sizeof(int)) == SUCCESS, "Dynamic array not initialized");
    for (int i = 0; i < sizeof(arr) / sizeof(int); i++) {
        ASSERT(ADD_DYN_ARR(&d, arr + i) == TRUE, "Couldn't add to dynamic array");
        ASSERT(SIZE_DYN_ARR(d) == i+1, "Expected size %d; got %d", i+1, SIZE_DYN_ARR(d));
        if (i < INITIAL_CAP)
            ASSERT(CAP_DYN_ARR(d) == INITIAL_CAP, 
                    "Expected capacity %d; got %d", INITIAL_CAP, CAP_DYN_ARR(d));
        else
            ASSERT(CAP_DYN_ARR(d) == 2 * INITIAL_CAP, 
                    "Expected capacity %d; got %d", 2 * INITIAL_CAP, CAP_DYN_ARR(d));

    }
    for (int i = 0; i < sizeof(arr) / sizeof(int); i++) {
        int v = GET_DYN_ARR(d, i, int);
        ASSERT(arr[i] == v, "At index %d: Expected %d; got %d", i, arr[i], v);
    }
    destroy_dyn_arr(&d);
    ASSERT(d.arr == NULL, "Dynamic array not freed");
    return 0;
}
