
#include <stdio.h>
#include "scheduler.h"

int task_function (void *arg) 
{
    int value = *((int*) arg);

    free(arg);
    printf("%d ", value);
    fflush(stdout);
    return 0;
}

int main (int argc, char *argv[])
{
    int failed, i;
    task_t *tptr;
    int *intptr;

    failed = task_scheduler_init();
    if (failed) {
        fprintf(stderr, "initialize_task_scheduler failed\n");
        return -1;
    }
    for (i = 0; i < 20; i++) {
        intptr = (int*) malloc(sizeof(int));
        *intptr = i;
        failed = task_schedule(30-i, 0, task_function, intptr, &tptr);
        if (failed) {
            fprintf(stderr, "schedule_task at iteration %d failed\n", i);
        }
    }
    while (1) sleep(1);
    return 0;
}








