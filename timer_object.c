
/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
**
** Author: Cihangir Metin Akyol, gee.akyol@gmail.com
** Copyright: Cihangir Metin Akyol, March 2016, November 2017
**
** This code is developed by and belongs to Cihangir Metin Akyol.  
** It is NOT owned by any company or consortium.  It is the sole
** property and work of one individual.
**
** It can be used by ANYONE or ANY company for ANY purpose as long 
** as ownership and/or patent claims are NOT made to it by ANYONE
** or any ANY ENTITY.
**
** It ALWAYS is and WILL remain the property of Cihangir Metin Akyol.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

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

#include <stdio.h>
#include "timer_object.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PUBLIC

PUBLIC void 
timer_report (timer_obj_t *tp, long long int iterations,
    double *returned_per_iteration_time_in_nsecs)
{
    long long int start_ns, end_ns, elapsed_ns;
    double elapsed_s, per_iter;

    start_ns = (tp->start.tv_sec * SEC_TO_NSEC_FACTOR) + tp->start.tv_nsec;
    end_ns = (tp->end.tv_sec * SEC_TO_NSEC_FACTOR) + tp->end.tv_nsec;
    elapsed_ns = end_ns - start_ns;
    elapsed_s = ((double) elapsed_ns) / ((double) (SEC_TO_NSEC_FACTOR));
    printf("elapsed time: %.9lf seconds (%llu nsecs) for %llu iterations\n",
        elapsed_s, elapsed_ns, iterations);
    per_iter = ((double) elapsed_ns / (double) iterations);
    printf("took %.3lf nano seconds per operation\n", per_iter);
    if (returned_per_iteration_time_in_nsecs) {
	*returned_per_iteration_time_in_nsecs = per_iter;
    }
}

#ifdef __cplusplus
} // extern C
#endif 



