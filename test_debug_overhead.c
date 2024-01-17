
#include <unistd.h>
#include "timer_object.h"
#include "debug_framework.h"

#define TEST			0
#define EXECUTE_ITERATIONS	((long long int) 10000000)
#define CHECK_ITERATIONS	(1500 * EXECUTE_ITERATIONS)

timer_obj_t tmr;

void do_nothing (char *message)
{
}

int main (int argc, char *argv[])
{
    volatile long long int i;
    double loop_overhead, total_overhead;

    debug_init();

    /* First measure the overhead of the for loop alone */
    printf("\nmeasuring the overhead of the for loop ...\n");
    timer_start(&tmr);
    for (i = 0; i < CHECK_ITERATIONS; i++) {
    }
    timer_end(&tmr);
    timer_report(&tmr, CHECK_ITERATIONS, &loop_overhead);

    printf("\ncalculating how much overhead is taken to only CHECK debug framework\n");
    debug_module_set_minimum_reporting_level(TEST, ERROR_LEVEL);
    timer_start(&tmr);
    for (i = 0; i < CHECK_ITERATIONS; i++) {
	    DEBUG(TEST, "should NEVER be printed");
    }
    timer_end(&tmr);
    printf("overhead of simply CHECKING debug framework\n");
    timer_report(&tmr, CHECK_ITERATIONS, &total_overhead);
    printf("exact overhead of debug checking: %.4lf nano seconds\n",
	total_overhead - loop_overhead);

    /* Now measure the overhead of reporting with an empty function */
    printf("\ncalculating how much overhead is taken to execute framework\n");
    debug_module_set_reporting_function(TEST, do_nothing);
    debug_module_set_minimum_reporting_level(TEST, DEBUG_LEVEL);
    timer_start(&tmr);
    for (i = 0; i < EXECUTE_ITERATIONS; i++) {
	    DEBUG(TEST, "should NEVER be printed");
    }
    timer_end(&tmr);
    printf("overhead of executing the debug framework\n");
    timer_report(&tmr, EXECUTE_ITERATIONS, NULL);

    return 0;
}


