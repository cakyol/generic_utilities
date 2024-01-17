
#include "list.h"

#define SIZE    (1024 * 8196)
#define ODD(i)  ((i & 1) == 1)
#define EVEN(i) ((i & 1) == 0)

byte array[SIZE];

static void
set_array (byte *array, int size, int value)
{
    int i;

    for (i = 0; i < size; i++) array[i] = value;
}

static void
reset_array (byte *array, int size)
{
    set_array(array, size, 0);
}

static void
ensure_all_are_set_to (byte *array, int size, int value)
{
    int i;

    for (i = 0; i < size; i++) {
        if (array[i] != value) {
            fprintf(stderr, "ALL check: array[%d] (%d) is NOT set to %d\n",
                    i, array[i], value);
        }
    }
}

static void
ensure_nothing_is_set (byte *array, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (array[i]) {
            fprintf(stderr,
                "EMPTY check: array[%d] (%d) should NOT have been set\n",
                i, array[i]);
        }
    }
}

static void
ensure_only_evens_are_set (byte *array, int size)
{
    int i;

    for (i = 0; i < size; i++) {

        /* the array entry with the even index should be set */
        if (EVEN(i)) {
            if (array[i] == 0) {
                fprintf(stderr,
                    "EVEN check: array[%d] SHOULD have been set\n", i);
            }
        } else {
            /* and the entry with the NON even index should NOT be set */
            if (array[i] != 0) {
                fprintf(stderr,
                    "EVEN check: array[%d] should NOT have been set\n", i);
            }
        }
    }
}

int main (int argc, char *argv[])
{
    list_t list;
    int i;
    void *ptr;
    list_node_t *node, *next_node;

    // debug_module_block_set_level(&list_debug, WARNING_DEBUG_LEVEL);

    if (list_init(&list, false, true, 0, null)) {
        fprintf(stderr, "LIST_INIT failed\n");
        return -1;
    }
    reset_array(array, SIZE);

    /* fill the list wth ALL entries */
    fprintf(stderr, "inserting all nodes\n");
    for (i = 0; i < SIZE; i++) {
        ptr = integer2pointer(i);
        if (list_prepend_data(&list, ptr, &node)) {
            fprintf(stderr, "LIST_ADD for %d failed\n", i);
        }
    }

    /* check whether all entries are in the list */
    fprintf(stderr, "checking all entries are present\n");
    node = list.head;
    while (node) {
        i = pointer2integer(node->data);
        if ((i >= 0) && (i < SIZE)) {
            array[i] = 1;
        } else {
            fprintf(stderr, "ALL check: index %d out of range (0-%d)\n",
                i, SIZE-1);
        }
        node = node->next;
    }
    fprintf(stderr, "now verifying all entries are in the list\n");
    ensure_all_are_set_to(array, SIZE, 1);

    /* now delete all odd values and check that even values still remain */
    fprintf(stderr, "removing all odd values now\n");
    node = list.head;
    while (node) {
        next_node = node->next;
        i = pointer2integer(node->data);
        if (ODD(i)) {
            list_remove_node(node);
            if ((i >= 0) && (i < SIZE)) {
                array[i] = 0;
            } else {
                fprintf(stderr, "ODD check: index %d out of range (0-%d)\n",
                    i, SIZE-1);
            }
        }
        node = next_node;
    }
    fprintf(stderr, "verifying that only even values are left\n");
    ensure_only_evens_are_set(array, SIZE);

    /* now delete all even values and check that nothing is left */
    fprintf(stderr, "removing all even values now\n");
    node = list.head;
    while ((node)) {
        next_node = node->next;
        i = pointer2integer(node->data);
        if (EVEN(i)) {
            list_remove_node(node);
            if ((i >= 0) && (i < SIZE)) {
                array[i] = 0;
            } else {
                fprintf(stderr, "EVEN check: index %d out of range (0-%d)\n",
                    i, SIZE-1);
            }
        }
        node = next_node;
    }
    ensure_nothing_is_set(array, SIZE);
    assert(list.n == 0);


    list_destroy(&list);
    return 0;
}



