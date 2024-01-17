
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "ordered_list.h"

#define START_INT   328
#define END_INT     4367
#define SPAN            (END_INT - START_INT + 1)

byte hashmap [END_INT + 5] = { 0 };

static void
flush (void)
{
    fflush(stdout);
    fflush(stderr);
    sleep(1);
}

int compare_pointers (void *ptr1, void *ptr2)
{
    return ((byte*) ptr1) - ((byte*) ptr2);
}

static int delete_count = 0;

void destroy_fn (void *user_data, void *arg)
{
    int value = pointer2integer(user_data);

    if ((value >= START_INT) && (value <= END_INT)) {
        hashmap[value] = 0;
        delete_count++;
        //printf("destroy for %d is called, delete_count %d\n",
                //value, delete_count);
    } else {
        fprintf(stderr, "destroy_fn received out of range value: %d\n", value);
    }
}

int main (int argc, char *argv[])
{
    int failed, failures;
    int result;
    int i;
    // int j, count;
    ordered_list_t sll;
    ordered_list_node_t *node;
    void *value;

    failed = ordered_list_init(&sll, 1, compare_pointers, NULL);
    if (failed) {
        fprintf(stderr, "ordered_list_init failed: %d\n", failed);
        return failed;
    } else {
        printf("list initialized\n");
    }
    flush();

    failures = 0;
    for (i = START_INT; i <= END_INT ; i++) {
        value = integer2pointer(i);

        /* FIRST time, it should succeed */
        failed = ordered_list_add_once(&sll, value, NULL);
        if (failed) {
            fprintf(stderr, "adding %d failed: %d\n", i, failed);
        }
        hashmap[i] = 1;

        /* consecutive ones should fail */
        failed = ordered_list_add_once(&sll, value, NULL);
        if (!failed) failures++;
        failed = ordered_list_add_once(&sll, value, NULL);
        if (!failed) failures++;
    }
    printf("all %d items added to list\n", sll.n);
    assert(sll.n == (END_INT - START_INT + 1));
    if (failures) {
        fprintf(stderr, "%d failures during repetitive inserts\n", failures);
    }
    for (i = START_INT; i <= END_INT ; i++) {
        if (hashmap[i] == 0) {
            fprintf(stderr, "hashmap %d did NOT get set\n", i);
        }
    }
    flush();

    printf("checking that the list is ordered .. ");
    node = sll.head;
    result = START_INT - 10;
    while (not_endof_ordered_list(node)) {
        //printf("%lld ", pointer2integer(node->user_data));
        if (pointer2integer(node->user_data) < result) {
            fprintf(stderr, "list order incorrect, current: %lld expected: >= %d\n",
                pointer2integer(node->user_data), result);
            result = pointer2integer(node->user_data);
        }
        node = node->next;
    }
    printf("ok\n");
    fflush(stdout);
    fflush(stdout);

    /* now delete one at a time and make sure everything is consistent */
    printf("now verifying deletions from the list\n");
    flush();

#if 0
    count = sll.n;
    int *s, *d;
    for (i = END_INT; i >= START_INT; i--) {

        failed = ordered_list_delete(&sll, integer2pointer(i), (void**) &d);
        if (failed == 0) {
            count--;
            assert(count == sll.n);
            assert(integer2pointer(i) == d);
            hashmap[i] = 0;

            /* 
             * make sure that the deleted entry is NOT in the list 
             * but ALL others are
             */
            for (j = START_INT; j <= END_INT; j++) {
                if (hashmap[j]) {
                    assert(ordered_list_search(&sll, integer2pointer(j), (void**)&s) == 0);
                    assert(j == pointer2integer(s));
                } else {
                    assert(ordered_list_search(&sll, integer2pointer(j), (void**)&s) == ENODATA);
                }
            }
        } else {
            fprintf(stderr, "ordered_list_delete for %d failed\n", i);
        }
    }
    assert(sll.n == 0);
    printf("all data verified, list is sane\n");
#endif /* 0 */

    delete_count = 0;
    ordered_list_destroy(&sll, destroy_fn, NULL);
    for (i = START_INT; i <= END_INT; i++) {
        if (hashmap[i]) {
            fprintf(stderr, "hashmap %d did NOT get cleared\n", i);
        }
    }
    printf("delete_count %d (should be %d)\n", delete_count, SPAN);

    return 0;
}


