
#include "utils.h"

#define MAX_SZ                  512

/*
** some data structure we are interested in
*/
typedef struct int_object_s {
    int value;
} int_object_t; 

int_object_t integers [MAX_SZ] = {{ 0 }};

int compareData (void *p1, void *p2)
{
    return
        ((int_object_t*) p1)->value - ((int_object_t*) p2)->value;
}

dll_manager_t list_object;
dll_manager_t *list = &list_object;

int verify_dll (dll_manager_t *list, int start, int increment)
{
    dll_element_t *elem;
    int_object_t *io;

    elem = list->head;
    while (elem) {
        io = (int_object_t*) elem->object;
        if (io->value != start) {
            printf("\nverification failed, expected %d got %d",
                start, io->value);
            return error;
        }
        start += increment;
        elem = elem->next;
    }
    return ok;
}

int main (argc, argv)
int argc;
char *argv [];
{
    int i, rc;
    int fault = false;

    /* initialize data array */
    for (i = 0; i < MAX_SZ; i++) integers[i].value = i;

    /* initialize dll list */
    dll_init(list, compareData);

    /* insert into the list the odd numbered values */
    printf("\nstarting odd numbers test");
    for (i = 0; i < MAX_SZ; i++) {
        if (integers[i].value & 1) {
            rc = dll_insert_object_ordered(list, &integers[i], true);
            if (rc == ok) {
                /* printf("    inserted %d\n", integers[i].value); */
            } else {
                fault = true;
                printf("\n    could NOT insert %d\n", integers[i].value);
            }
            fflush(stdout);
        }
    }
    if (fault) {
        printf("\nabandoning rest of test\n");
        return -1;
    }

    /* now verify that only the odd ones are present */
    if (verify_dll(list, 1, 2) == ok) {
        printf("\nodd numbers test passed");
    }

    /* now add even numbers to list and check */
    printf("\nnow adding the even numbers too");
    for (i = 0; i < MAX_SZ; i++) {
        if (0 == (integers[i].value & 1)) {
            rc = dll_insert_object_ordered(list, &integers[i], true);
            if (rc != ok) {
                fault = true;
                printf("\n    could NOT insert %d\n", integers[i].value);
                fflush(stdout);
            }
        }
    }
    if (fault) {
        printf("\nabandoning rest of test\n");
        return -1;
    }

    /* now verify ALL are present */
    if (verify_dll(list, 0, 1) == ok) {
        printf("\nall numbers test passed");
    }

    /* now delete all the odd ones */
    printf("\nnow deleting all odd numbers");
    for (i = 0; i < MAX_SZ; i++) {
        if (integers[i].value & 1) {
            dll_delete_object(list, &integers[i]);
        }
    }
    if (verify_dll(list, 0, 2) == ok) {
        printf("\nall odd number deletions verified");
    }

    /* now delete all the even ones, meaning all is now deleted */
    printf("\nnow deleting all even numbers");
    for (i = 0; i < MAX_SZ; i++) {
        if (0 == (integers[i].value & 1)) {
            dll_delete_object(list, &integers[i]);
            if (dll_find_object(list, &integers[i], NULL)) {
                printf("\nERROR: deleted %d and found it", integers[i].value);
            }
        }
    }
    if (list->count == 0) {
        printf("\nall number deletions verified");
    }

    printf("\n");
    return 0;
}

