
#include <stdio.h>

#include "timer_object.h"
#include "avl_tree_object.h"

#define MAX_SZ          (50 * 1024 * 1024)
#define MAGIC           12344321
#define ITER            5

int data [MAX_SZ];
timer_obj_t timr;
int trav_cnt = 0;

int int_compare (void *p1, void *p2)
{ 
    int *i1 = p1;
    int *i2 = p2;
    return i1 - i2;
}

static int
travfn (void *utility, void *node, void *data,
        void *p0, void *p1, void *p2, void *p3)
{
    int *addr = (int*) data;

    if (*addr != 0) printf("ERROR, expected 0 got %d\n", *addr);
    *addr = pointer2integer(addr);
    trav_cnt++;
    return 0;
}

static void
destroy (void *data, void *unused)
{
    int *ip = (int*) data;
    *ip = 0;
}

static void
traverse_test (void)
{
    int iter, i, correct;
    avl_tree_t tree;
    void *found;
    timer_obj_t tmr;
    long long int bytes;
    double mbytes;

    avl_tree_init(&tree, true, false, int_compare, NULL);
    printf("inserting %d pieces of data .. ", MAX_SZ); fflush(stdout);
    timer_start(&tmr);
    for (i = 0; i < MAX_SZ; i++) {
        data[i] = 0;
        if (avl_tree_insert(&tree, &data[i], &found, false)) {
            printf("avl_tree_insert failed at iter %d, data %p\n",
                    i, &data[i]);
        }
        if (found) printf("found %p at iter %d\n", found, i);
    }
    timer_end(&tmr);
    printf("ok\n");
    timer_report(&tmr, MAX_SZ, NULL);

    /* verify all entries are there */
    printf("\nnow verifying all data in array .. "); fflush(stdout);
    timer_start(&tmr);
    for (i = 0; i < MAX_SZ; i++) {
        if (avl_tree_search(&tree, &data[i], NULL)) {
            printf("iter %d data %p not found\n",
                    i, &data[i]);
        }
    }
    timer_end(&tmr);
    printf("ok\n");
    timer_report(&tmr, MAX_SZ, NULL);

    /* now traverse */
    for (iter = 0; iter < ITER; iter++) {
        trav_cnt = 0;
        for (i = 0; i < MAX_SZ; i++) data[i] = 0;
        timer_start(&tmr);
        printf("\ntraversing iteration %d .. ", iter); fflush(stdout);
        avl_tree_iterate(&tree, NULL, travfn, null, null, null, null);
        timer_end(&tmr);
        printf("ok\n"); fflush(stdout);
        timer_report(&tmr, trav_cnt, NULL);

        printf("\nnow verifying traversal .. "); fflush(stdout);
        for (i = 0, correct = 0; i < MAX_SZ; i++) {
            if (data[i] != pointer2integer(&data[i])) {
                printf("data at index %d is not %d (%d)\n",
                        i, (int) pointer2integer(&data[i]), data[i]);
            } else {
                correct++;
            }
        }
        printf("ok\n"); fflush(stdout);
        printf("i = %d, correct = %d\n", i, correct);
    }


    /* test destruction */
    OBJECT_MEMORY_USAGE(&tree, bytes, mbytes);
    printf("\ntotal memory = %lld bytes %f Mbytes",
            bytes, mbytes);
    printf("\nnow destroying .. "); fflush(stdout);
    for (i = 0; i < MAX_SZ; i++) {
        data[i] = 12345;
    }
    avl_tree_destroy(&tree, destroy, NULL);
    for (i = 0; i < MAX_SZ; i++) {
        if (data[i]) printf("element %d not destroyed: %d\n",
            i, data[i]);
    }
    printf("ok\n");
}

#if 0

void perform_avl_tree_test (avl_tree_t *avlt, int use_odd_numbers)
{
    int i, lo, hi, d, fail_search;
    int failed;
    int fine, not_fine;
    char *oddness = use_odd_numbers ? "odd" : "even";
    char *reverse = use_odd_numbers ? "even" : "odd";
    unsigned long long int bytes_used;
    double megabytes_used;
    void *fwdata, *searched, *found, *removed;

    printf("size of ONE avl node is: %lu bytes\n",
            sizeof(avl_node_t));

    for (i = 0; i < MAX_SZ; i++) {
    }

    /* 
    ** Fill opposing ends of array, converge in middle.
    ** This gives some sort of randomness to data.
    */
    printf("filling array of size %d with %s number data\n", 
        MAX_SZ, oddness);
    d = use_odd_numbers ? 1 : 0;
    lo = 0; 
    hi = MAX_SZ - 1;
    while (1) {
        data[lo++] = d;
        d += 2;
        data[hi--] = d;
        d += 2;
        if (lo > hi) break;
    }

    if (max_value_reached < d) {
        max_value_reached = d + 10;
        printf("max value recorded so far is %d\n", max_value_reached);
    }

    avl_tree_init(avlt, true, int_compare, NULL);

    /* enter all array data into avl tree */
    printf("now entering all %s number data into the avl tree\n", oddness);
    timer_start(&timr);
    for (i = 0; i < MAX_SZ; i++) {
        fwdata = &data[i];
        failed = avl_tree_insert(avlt, fwdata, &found, false);
        if (failed != 0) {
            printf("populate_data: avl_tree_insert error: %d failed\n", i);
        }
    }
    timer_end(&timr);
    timer_report(&timr, MAX_SZ, NULL);
    OBJECT_MEMORY_USAGE(avlt, bytes_used, megabytes_used);
    printf("total memory used by the avl tree of %d nodes: %llu bytes (%lf Mbytes)\n",
        avlt->n, bytes_used, megabytes_used);

    printf("searching for non existant data\n");
    fine = not_fine = 0;
    timer_start(&timr);
    for (i = max_value_reached; i < (max_value_reached + EXTRA); i++) {
        searched = &i;
        failed = avl_tree_search(avlt, searched, &found);
        if ((failed == 0) || found) {
            not_fine++;
        } else {
            fine++;
        }
    }
    timer_end(&timr);
    timer_report(&timr, EXTRA, NULL);
    printf("expected %d, NOT expected %d\n", fine, not_fine);

    /* now search all data that should be found (all of them) */
    printf("now searching all %s numbers in the avl tree\n", oddness);
    fine = not_fine = 0;
    timer_start(&timr);
    for (i = 0; i < MAX_SZ; i++) {
        searched = &data[i];
        failed = avl_tree_search(avlt, searched, &found);

        if ((failed != 0) || (data[i] != *((int*) found))) {
            not_fine++;
        } else {
            fine++;
        }
    }
    timer_end(&timr);
    timer_report(&timr, MAX_SZ, NULL);
    printf("found %d as expected and %d as NOT expected\n", fine, not_fine);

    /* now search for all entries that should NOT be in the tree */
    printf("now searching for all %s numbers in the avl tree\n", reverse);
    fine = not_fine = 0;
    d = use_odd_numbers ? 0 : 1;
    timer_start(&timr);
    for (i = 0; i < MAX_SZ; i++) {
        searched = &d;
        failed = avl_tree_search(avlt, searched, &found);
        if ((failed == 0) || found) {
            not_fine++;
        } else {
            fine++;
        }
        d += 2;
    }
    timer_end(&timr);
    timer_report(&timr, MAX_SZ, NULL);
    printf("%d as expected and %d as NOT expected\n", fine, not_fine);

int tree_nodes = avlt->n;
printf("now deleting the whole tree (%d nodes)\n", tree_nodes);
timer_start(&timr);
avl_tree_destroy(avlt);
timer_end(&timr);
timer_report(&timr, tree_nodes, NULL);
return;

    avl_tree_destroy(avlt, user_data_destroy_function, NULL);
    return;

    /* now delete one entry at a time and search it (should not be there) */
    printf("now deleting and searching\n");
    fine = not_fine = fail_search = 0;
    timer_start(&timr);
    for (i = 0; i < MAX_SZ; i++) {
        searched =  &data[i];
        failed = avl_tree_remove(avlt, searched, &removed);
        if ((failed != 0) || (data[i] != *((int*) removed))) {
            not_fine++;
        } else {
            fine++;
        }
        failed = avl_tree_search(avlt, searched, &found);
        if ((failed == 0) || found) {
            fail_search++;
        }
    }
    timer_end(&timr);
    timer_report(&timr, MAX_SZ*2, NULL);
    printf("deleted %d, could NOT delete %d, erroneous search %d\n",
        fine, not_fine, fail_search);

    OBJECT_MEMORY_USAGE(avlt, bytes_used, megabytes_used);
    printf("total memory used by the avl tree (%d nodes) after deletes: "
        "%llu bytes (%f Mbytes)\n",
        avlt->n, bytes_used, megabytes_used);

    avl_tree_destroy(avlt, user_data_destroy_function, NULL);
}

#endif // 0

int main (argc, argv)
int argc;
char *argv [];
{
    traverse_test();
    return 0;

#if 0
    perform_avl_tree_test(&avlt, 1);
    printf("\n\n");
    perform_avl_tree_test(&avlt, 0);
    
    return 0;
#endif // 0
}

