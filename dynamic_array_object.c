
/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
**
** Author: Cihangir Metin Akyol (gee.akyol@gmail.com, gee_akyol@yahoo.com)
** Copyright: Cihangir Metin Akyol, March 2016
**
** This code is developed by and belongs to Cihangir Metin Akyol.  
** It is NOT owned by any company or consortium.  It is the sole
** property and work of one individual.
**
** It can be used by ANYONE or ANY company for ANY purpose as long 
** as NO ownership and/or patent claims are made to it by such persons 
** or companies.
**
** It ALWAYS is and WILL remain the property of Cihangir Metin Akyol.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "dynamic_array_object.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DYNAMIC_ARRAY_INITIAL_SIZE      16

static int
dynamic_array_grow_negative (dynamic_array_t *datp, int new_lowest)
{
    int i, start, end, old_lowest;
    int old_size, new_size, size_diff;
    void **new_elements;

    /* set some helper variables */
    old_lowest = datp->lowest;
    old_size = datp->highest - old_lowest + 1;
    new_size = old_size + (old_lowest - new_lowest);
    size_diff = new_size - old_size;

    /* allocate the new array */
    new_elements = MEM_MONITOR_ZALLOC(datp, new_size);
    if (NULL == new_elements) return ENOMEM;

    /* copy the existing data into the new array and set parameters */
    start = old_lowest - new_lowest;
    end = start + old_size - 1;
    for (i = start; i <= end; i++)
        new_elements[i] = datp->elements[i - size_diff];
    datp->elements = new_elements;
    datp->lowest = new_lowest;

    /* release te old memory */
    MEM_MONITOR_FREE(datp->elements);

    return 0;
}

static int
dynamic_array_grow_positive (dynamic_array_t *datp, int new_highest)
{
    int i, old_lowest, old_highest;
    int old_size, new_size;
    void **new_elements;

    old_lowest = datp->lowest;
    old_highest = datp->highest;
    old_size = old_highest - old_lowest + 1;
    new_size = old_size + (new_highest - old_highest);
    new_elements = MEM_MONITOR_ZALLOC(datp, new_size);
    if (NULL == new_elements) return ENOMEM;
    for (i = datp->lowest; i <= datp->highest; i++)
        new_elements[i - old_lowest] = datp->elements[i - old_lowest];
    MEM_MONITOR_FREE(datp->elements);
    datp->elements = new_elements;
    datp->highest = new_highest;

    return 0;
}
 
static int
thread_unsafe_dynamic_array_insert (dynamic_array_t *datp,
        int index, void *data)
{
    int error = 0;

    if (index < datp->lowest) {
        error = dynamic_array_grow_negative(datp, index);
    } else if (index > datp->highest) {
        error = dynamic_array_grow_positive(datp, index);
    }
    if (error) {
        insertion_failed(datp);
        return error;
    }
    datp->elements[index - datp->lowest] = data;
    insertion_succeeded(datp);

    return 0;
}

static void *
thread_unsafe_dynamic_array_get (dynamic_array_t *datp, int index)
{
    if ((index >= datp->lowest) && (index <= datp->highest)) {
        search_succeeded(datp);
        return
            datp->elements[index - datp->lowest];
    }
    search_failed(datp);
    return NULL;
}

static int
thread_unsafe_dynamic_array_delete (dynamic_array_t *datp, int index)
{
    if ((index >= datp->lowest) && (index <= datp->highest)) {
        datp->elements[index - datp->lowest] = NULL;
        deletion_succeeded(datp);
        return 0;
    }
    deletion_failed(datp);
    return EFAULT;
}

/***************************** PUBLIC *************************************/

PUBLIC int
dynamic_array_init (dynamic_array_t *datp,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        int initial_size,
        mem_monitor_t *parent_mem_monitor)
{
    MEM_MONITOR_SETUP(datp);
    LOCK_SETUP(datp);
    STATISTICS_SETUP(datp);

    if (0 >= initial_size) return EINVAL;
    datp->elements = MEM_MONITOR_ZALLOC(datp,
                        (initial_size * sizeof(void*)));
    if (NULL == datp->elements) return ENOMEM;
    datp->lowest = 0;
    datp->highest = initial_size - 1;
    reset_stats(datp);

    return 0;
}

PUBLIC int
dynamic_array_insert (dynamic_array_t *datp, int index, void *data)
{
    int failed;

    OBJ_WRITE_LOCK(datp);
    failed = thread_unsafe_dynamic_array_insert(datp, index, data);
    OBJ_WRITE_UNLOCK(datp);
    return failed;
}

PUBLIC void *
dynamic_array_get (dynamic_array_t *datp, int index)
{
    void *data;

    OBJ_READ_LOCK(datp);
    data = thread_unsafe_dynamic_array_get(datp, index);
    OBJ_READ_UNLOCK(datp);
    return data;
}

PUBLIC int
dynamic_array_delete (dynamic_array_t *datp, int index)
{
    int failed;

    OBJ_WRITE_LOCK(datp);
    failed = thread_unsafe_dynamic_array_delete(datp, index);
    OBJ_WRITE_UNLOCK(datp);
    return failed;
}

PUBLIC void 
dynamic_array_destroy (dynamic_array_t *datp,
        destruction_handler_t dcbf, void *extra_arg)
{
    int i, idx;

    OBJ_WRITE_LOCK(datp);
    if (datp->elements) {
        if (dcbf) {
            for (i = datp->lowest; i <= datp->highest; i++) {
                idx = i - datp->lowest;
                if (datp->elements[idx]) {
                    dcbf(datp->elements[idx], extra_arg);
                }
            }
        }
        MEM_MONITOR_FREE(datp->elements);
    }
    OBJ_WRITE_UNLOCK(datp);
    LOCK_OBJ_DESTROY(datp);
    memset(datp, 0, sizeof(dynamic_array_t));
}

#ifdef __cplusplus
} // extern C
#endif 








