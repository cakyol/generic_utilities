
#include <stdio.h>
#include <assert.h>

#include "qobject.h"
#include "timer_object.h"

#define QUEUE_SIZE                      10000
#define QUEUE_EXPANSION_INCREMENT       8000
#define ITER_COUNT                      (QUEUE_SIZE * 50)

int main (int argc, char *argv[])
{
    qobj_t qobj;
    int i, j, n_stored, mismatched;
    long long int bytes;
    double mbytes;
    timer_obj_t timr;
    void *pointer;

    if (qobj_init(&qobj, 1,
                sizeof(void*),
                QUEUE_SIZE,
                QUEUE_EXPANSION_INCREMENT, NULL)) {
            fprintf(stderr, "qobj_init failed\n");
            return -1;
    }

    /* fill up the fifo */
    timer_start(&timr);
    printf("Populating the queue\n");
    fflush(stdout);
    for (i = 0; i < ITER_COUNT; i++) {
        pointer = integer2pointer(i);
        if (qobj_queue(&qobj, &pointer, NULL)) {
            fprintf(stderr, "queueing %d failed\n", i);
            return -1;
        }
    }
    timer_end(&timr);
    timer_report(&timr, ITER_COUNT, NULL);

    n_stored = qobj.n;
    OBJECT_MEMORY_USAGE(&qobj, bytes, mbytes);

    /* now read back and verify */
    mismatched = i = 0;
    printf("Now dequeuing & verifying\n");
    timer_start(&timr);
    while (0 == qobj_dequeue(&qobj, &pointer, NULL)) {
        j = pointer2integer(pointer);
        if (j == i) {
            // printf("queue data %d %d verified\n", i, j);
        } else {
            fprintf(stderr, "dequeue data mismatch: dqed %d, should be %d\n",
                j, i);
            mismatched++;
        }
        i++;
    }
    timer_end(&timr);
    timer_report(&timr, i, NULL);
    assert((qobj.n == 0) && (i == n_stored) && (mismatched == 0));
    printf("\nqueue object is sane\n  capacity %d\n  expanded %d times\n"
            "  memory %lld bytes %f mbytes\n",
        qobj.maximum_size, qobj.expansion_count, bytes, mbytes);
    return 0;
}



