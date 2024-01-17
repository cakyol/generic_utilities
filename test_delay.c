
#include <stdio.h>
#include "timer_object.h"

int main (int argc, char *argv[])
{
    timer_obj_t tmr;

    while (1) {
        timer_start(&tmr);
        nano_seconds_sleep(999999999);
        timer_end(&tmr);
        printf("%ld seconds %ld nanoseconds elapsed (%lld)\n",
            tmr.end.tv_sec - tmr.start.tv_sec,
            tmr.end.tv_nsec - tmr.start.tv_nsec,
            timer_delay_nsecs(&tmr));
    }
}

