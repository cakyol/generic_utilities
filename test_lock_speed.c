
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "timer_object.h"
#include "lock_object.h"

#define MAX_THREADS         (16 * 1024)
#define MAX_ITERATION       10000

lock_obj_t lock;
lock_obj_t protect_globals;
char thread_complete_array [MAX_THREADS] = { 0 };
int max_threads = 0;
volatile int start_timing = 0;
nano_seconds_t total_time = 0;
long long int total_iterations = 0;

void *contention_thread (void *arg)
{
    timer_obj_t tmr;
    int i, tid = *((int*) arg);

    free(arg);

    /* wait until let loose */
    while (start_timing == 0);

    /* fight over the common lock */
    timer_start(&tmr);
    for (i = 0; i < MAX_ITERATION; i++) {
        grab_write_lock(&lock);
        release_write_lock(&lock);
    }
    timer_end(&tmr);

    /* update the time taken & total number of iterations */
    grab_write_lock(&protect_globals);
    total_iterations += MAX_ITERATION;
    total_time += timer_delay_nsecs(&tmr);
    release_write_lock(&protect_globals);

    /* clean up & exit */
    thread_complete_array[tid] = 1;

    /* done */
    return NULL;
}

int main (int argc, char *argv[])
{
    int failed;
    pthread_t tid;
    int i;
    int *intp;

    lock_obj_init(&lock);
    lock_obj_init(&protect_globals);

    /* create all the threads until no more can be created */
    printf("creating all threads\n");
    start_timing = 0;
    for (i = 0; i < MAX_THREADS; i++) {
        intp = (int*) malloc(sizeof(int));
        if (NULL == intp) break;
        *intp = i;
        failed = pthread_create(&tid, NULL, contention_thread, intp);
        if (failed) break;
        max_threads++;
    }
    printf("created %d of them, now letting them loose on the lock\n",
        max_threads);

    /* ok all threads are fired up & waiting, let them loose */
    start_timing = 1;

    /* now wait until they all complete */
not_all_threads_complete:
    for (i = 0; i < max_threads; i++) {
        if (thread_complete_array[i] == 0) {
            sleep(1);
            goto not_all_threads_complete;
        }
    }

    /* everything should be updated now */
    printf("%lld nano seconds used in %lld iterations, average is %lld\n",
        total_time, total_iterations, (total_time/total_iterations));
    return 0;
}




