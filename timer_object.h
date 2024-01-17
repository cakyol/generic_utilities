
/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
** 
** @@@@@ Time measurement functions
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __TIMER_OBJ_H__
#define __TIMER_OBJ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include <time.h>

/* seconds to milli seconds multiplier */
#define SEC_TO_MSEC_FACTOR                      (1000)

/* seconds to micro seconds multiplier */
#define SEC_TO_USEC_FACTOR                      (1000000LL)

/* seconds to nano seconds multiplier */
#define SEC_TO_NSEC_FACTOR                      (1000000000LL)

typedef long long int nano_seconds_t;
typedef long long int micro_seconds_t;
typedef struct timespec timespec_t;
typedef struct timeval timeval_t;
typedef struct itimerval itimerval_t;

typedef struct timer_obj_s {
    timespec_t start;
    timespec_t end;
} timer_obj_t;

/* Big Sur does not need this */
#if 0
#ifdef __APPLE__

#define CLOCK_MONOTONIC         1

static inline int
clock_gettime (int clock_id, timespec_t *ts)
{
    timeval_t tv;

    if (gettimeofday(&tv, NULL) < 0) return -1;
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;

    return 0;
}

#endif // __APPLE__
#endif // 0

static inline void 
timer_start (timer_obj_t *tp)
{ clock_gettime(CLOCK_MONOTONIC, &tp->start); }

static inline void 
timer_end (timer_obj_t *tp)
{ clock_gettime(CLOCK_MONOTONIC, &tp->end); }

/*
 * returns the time difference (in nano seconds) between
 * when 'timer_end' and 'timer_start' were called.
 */
static inline nano_seconds_t
timer_delay_nsecs (timer_obj_t *tp)
{
    return
        (tp->end.tv_sec * SEC_TO_NSEC_FACTOR) + tp->end.tv_nsec -
        (tp->start.tv_sec * SEC_TO_NSEC_FACTOR) - tp->start.tv_nsec;
}

/*
 * returns an arbitrary reference point in time to base all consecutive
 * relative times from.  Typically, at boot time, it may return 0,
 * and every time it is called therefater, it will return the time
 * passed (in nano seconds) relative to that first call.
 */
static inline nano_seconds_t
time_now (void)
{
    timespec_t now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return
        (now.tv_sec * SEC_TO_NSEC_FACTOR) + now.tv_nsec;
}

/*
 * Sleep for a pre determined nano seconds specified in the parameter.
 * Do **NOT** pass a parameter greater than one second equivalent as
 * the parameter, ie the max valid value is 999,999,999 (almost 1 second).
 */
static inline void
nano_seconds_sleep (nano_seconds_t nsecs)
{
    timespec_t n;

    n.tv_sec = 0;
    n.tv_nsec = nsecs;
    nanosleep(&n, 0);
}

/*
 * just a printf convenience
 */
extern 
void timer_report (timer_obj_t *tp, long long int iterations,
    double *returned_per_iteration_time_in_nsecs);

#ifdef __cplusplus
} // extern C
#endif 

#endif // __TIMER_OBJ_H__




