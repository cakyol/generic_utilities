
#include <stdio.h>
#include "stack_object.h"

#define ITERATION       10000
#define STACK_SIZE      1000
#define STACK_GROWTH    500

int array [ITERATION];

int main (int argc, char *argv[])
{
    stack_obj_t stk;
    int i, *returned_data;

    if (stack_obj_init(&stk, 1, STACK_SIZE, STACK_GROWTH, NULL)) {
        fprintf(stderr, "stack_obj_init failed\n");
        return -1;
    }

    /* fill the array */
    for (i = 0; i < ITERATION; i++) array[i] = i * 3;

    /* push all data */
    for (i = 0; i < ITERATION; i++) {
        if (stack_obj_push(&stk, &array[i]) != 0) {
            fprintf(stderr, "stack_obj_push_integer of index %d failed\n", i);
            return -1;
        }
    }

    /* now pop them & verify */
    for (i = ITERATION-1; i >= 0; i--) {
        if (stack_obj_pop(&stk, (void**) &returned_data) != 0) {
            fprintf(stderr, "stack_obj_pop_integer of iteration %d failed\n", i);
            return -1;
        }
        if (*returned_data != (i * 3)) {
            fprintf(stderr, "returned data %d doesnt match %d for iteration %d\n",
                *returned_data, i*3, i);
        }
    }

    printf("stack object works fine; expanded %d times\n", stk.expansion_count);
    return 0;
}



