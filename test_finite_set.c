
#include "utils.h"

#define ITER_MAX        6
#define MAX_SZ          (100 * MILLION)
#define DATA_GAP        5
double double_time;
finite_set_t fs;

void do_test (void)
{
    boolean bfailed;
    int i, data, ref;
    int iter;
    struct timeval start, end, elapsed;
    void *ud;

    /* create */

    printf("size of one node = %d bytes\n", (int) sizeof(finite_set_node_t));

    if (ok != finite_set_init(&fs, TRUE, MAX_SZ, NULL)) {
        fprintf(stderr, "finite_set_init failed in do_test\n");
        return;
    }

    /* populate keys in finite set */
    for (i = 0, data = 0; i < MAX_SZ; i++, data += DATA_GAP)
        finite_set_store_key(&fs, i, data);

    printf("\nbeginning test\n");

    /* insert/populate */

    gettimeofday(&start, NULL);
    for (iter = 0; iter < ITER_MAX; iter++) {
        data = 0;
        for (i = 0; i < MAX_SZ; i++) {
            bfailed = finite_set_insert(&fs, data, NULL, &ref);
            if (bfailed || (ref != (iter+1))) {
                fprintf(stderr, 
                    "finite_set_insert failed in do_test; iteration %d\n", i);
            }
            data += DATA_GAP;
        }
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &elapsed);
    double_time = (elapsed.tv_sec * MILLION) + elapsed.tv_usec;
    printf("\n\n");
    printf("total finite set insert time: %.3lf usecs\n", double_time);
    printf("average finite set insert time: %.3lf usecs\n", 
        double_time/MAX_SZ/ITER_MAX);

    /* search */

    gettimeofday(&start, NULL);
    for (iter = 0; iter < ITER_MAX; iter++) {
        data = 0;
        for (i = 0; i < MAX_SZ; i++) {
            bfailed = finite_set_search(&fs, data, &ud, &ref);
            if (bfailed || (ref != ITER_MAX)) {
                fprintf(stderr, 
                    "finite_set_search failed in do_test; iteration %d\n", i);
            }
            data += DATA_GAP;
        }
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &elapsed);
    double_time = (elapsed.tv_sec * MILLION) + elapsed.tv_usec;
    printf("\n\n");
    printf("total finite set search time: %.3lf usecs\n", double_time);
    printf("average finite set search time: %.3lf usecs\n", 
        double_time/MAX_SZ/ITER_MAX);

    /* delete */

    gettimeofday(&start, NULL);
    int removed, remaining;
    for (iter = 0; iter < ITER_MAX; iter++) {
        data = 0;
        for (i = 0; i < MAX_SZ; i++) {
            bfailed = finite_set_remove(&fs, data, 1, &ud, &removed, &remaining);
            if (bfailed || (removed != 1) || (remaining != (ITER_MAX - iter - 1))) {
                fprintf(stderr, 
                    "finite_set_remove failed in do_test; iteration %d\n", i);
            }
            data += DATA_GAP;
        }
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &elapsed);
    double_time = (elapsed.tv_sec * MILLION) + elapsed.tv_usec;
    printf("\n\n");
    printf("total finite set delete time: %.3lf usecs\n", double_time);
    printf("average finite set delete time: %.3lf usecs\n", 
        double_time/MAX_SZ/ITER_MAX);

    printf("\n\n");
}

int main (int argc, char *argv[])
{
    do_test();
    return 0;
}

