
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

#include "index_object.h"

#ifdef __cplusplus
extern "C" {
#endif

static int
index_obj_resize (index_obj_t *idx, int new_size)
{
    void **new_elements;

    new_elements = MEM_MONITOR_ZALLOC(idx, new_size * sizeof(void*));
    if (NULL == new_elements) return ENOMEM;
    copy_pointer_blocks(idx->elements, new_elements, idx->n);
    MEM_MONITOR_FREE(idx->elements);
    idx->elements = new_elements;
    idx->maximum_size = new_size;
    return 0;
}

/*
** This is the heart of the object: binary search
*/
static int 
index_obj_find_position (index_obj_t *idx,
        void *searched_data, 
        int *insertion_point)
{
    register int mid, diff, lo, hi;

    lo = mid = diff = 0;
    hi = idx->n - 1;

    /* binary search */
    while (lo <= hi) {
        mid = (hi+lo) >> 1;
        diff = (idx->cmpf)(searched_data, idx->elements[mid]);
        if (diff > 0) {
            lo = mid + 1;
        } else if (diff < 0) {
            hi = mid - 1;
        } else {
            return mid;
        }
    }

    /*
    ** not found, but record where the element should be 
    ** inserted, in case it was required to be put into 
    ** the array.
    */
    safe_pointer_set(insertion_point, (diff > 0 ? (mid + 1) : mid));
    return -1;
}

static int
thread_unsafe_index_obj_insert (index_obj_t *idx,
        void *data,
        void **present_data,
        boolean overwrite_if_present)
{
    int insertion_point = 0;    /* shut the -Werror up */
    int size, i;
    void **source;

    /* assume no entry */
    safe_pointer_set(present_data, NULL);

    /* being traversed, cannot be changed */
    if (idx->should_not_be_modified) {
        insertion_failed(idx);
        return EBUSY;
    }

    /*
    ** see if element is already there and if not,
    ** note the insertion point in "insertion_point".
    */
    i = index_obj_find_position(idx, data, &insertion_point);

    /* key/data already in index */
    if (i >= 0) {
        safe_pointer_set(present_data, idx->elements[i]);
        if (overwrite_if_present) {
            idx->elements[i] = data;
            insertion_succeeded(idx);
        }
        return 0;
    }

    /* if index is full, attempt to expand by specified expansion_size */
    if (idx->n >= idx->maximum_size) {

        /* cannot expand, not allowed */
        if (idx->expansion_size <= 0) {
            insertion_failed(idx);
            return ENOSPC;
        }

        /* tried to expand but failed */
        if (index_obj_resize(idx, idx->maximum_size + idx->expansion_size)) {
            insertion_failed(idx);
            return ENOMEM;
        }
    }

    /*
    ** shift all of the pointers after 
    ** "insertion_point" right by one 
    */
    source = &(idx->elements[insertion_point]);
    if ((size = idx->n - insertion_point) > 0)
        copy_pointer_blocks(source, (source+1), size);
    
    /* fill in the new node values */
    idx->elements[insertion_point] = data;

    /* increment element count */
    idx->n++;

    insertion_succeeded(idx);

    return 0;
}

static int 
thread_unsafe_index_obj_search (index_obj_t *idx,
        void *data,
        void **found, int *index_found_at)
{
    int i;

    safe_pointer_set(found, NULL);
    safe_pointer_set(index_found_at, -1);

    i = index_obj_find_position(idx, data, NULL);

    /* not found */
    if (i < 0) {
        search_failed(idx);
        return ENODATA;
    }

    safe_pointer_set(found, idx->elements[i]);
    safe_pointer_set(index_found_at, i);
    search_succeeded(idx);
    return 0;
}

/*
 * You have to know exactly what you are doing with this function.
 * You can replace a data ONLY if you ALREADY know that the ordering
 * of the data is not different with the new given data.
 */
static int
thread_unsafe_index_obj_replace (index_obj_t *idx,
        int i, void *new_data)
{
    if ((i < 0) || (i >= idx->n)) return EINVAL;
    idx->elements[i] = new_data;
    return 0;
}

static int
thread_unsafe_index_obj_remove (index_obj_t *idx,
        void *data,
        void **data_removed,
        int shrink_threshold)
{
    int i, size;

    safe_pointer_set(data_removed, NULL);

    if (idx->should_not_be_modified) {
        deletion_failed(idx);
        return EBUSY;
    }

    /* first see if it is there */
    i = index_obj_find_position(idx, data, NULL);

    /* not in table */
    if (i < 0) {
        deletion_failed(idx);
        return ENODATA;
    }

    safe_pointer_set(data_removed, idx->elements[i]);
    idx->n--;

    /* pull the elements AFTER "index" to the left by one */
    if ((size = idx->n - i) > 0) {
        void **source = &idx->elements[i+1];
        copy_pointer_blocks(source, (source - 1), size);
    }

    deletion_succeeded(idx);

    /*
     * Now if shrinking of the index is needed & possible, do it.
     * If this fails, no harm is done.
     */
    if (shrink_threshold > 0) {
        if (idx->maximum_size > (idx->n + shrink_threshold)) {
            (void) index_obj_resize(idx, idx->n + shrink_threshold);
        }
    }

    return 0;
}

/**************************** Initialize *************************************/

PUBLIC int
index_obj_init (index_obj_t *idx,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        object_comparer cmpf,
        int maximum_size,
        int expansion_size,
        mem_monitor_t *parent_mem_monitor)
{
    int failed = 0;

    if ((maximum_size <= 1) || (expansion_size < 0)) {
        return EINVAL;
    }

    MEM_MONITOR_SETUP(idx);
    LOCK_SETUP(idx);
    STATISTICS_SETUP(idx);

    idx->should_not_be_modified = false;
    idx->maximum_size = maximum_size;
    idx->expansion_size = expansion_size;
    idx->cmpf = cmpf;
    idx->n = 0;
    reset_stats(idx);
    idx->elements = MEM_MONITOR_ZALLOC(idx, sizeof(void*) * maximum_size);
    if (NULL == idx->elements) {
        failed = ENOMEM;
    }

    return failed;
}

/**************************** Insert *****************************************/

PUBLIC int
index_obj_insert (index_obj_t *idx,
        void *data,
        void **present_data,
        boolean overwrite_if_present)
{
    int failed;

    OBJ_WRITE_LOCK(idx);
    failed = thread_unsafe_index_obj_insert(idx, data,
                present_data, overwrite_if_present);
    OBJ_WRITE_UNLOCK(idx);
    return failed;
}

/**************************** Search *****************************************/

PUBLIC int
index_obj_search (index_obj_t *idx,
        void *data,
        void **present_data, int *index_found_at)
{
    int failed;

    OBJ_READ_LOCK(idx);
    failed = thread_unsafe_index_obj_search(idx, data,
                present_data, index_found_at);
    OBJ_READ_UNLOCK(idx);
    return failed;
}

PUBLIC int
index_obj_replace (index_obj_t *idx,
        int index, void *new_data)
{
    int failed;

    OBJ_WRITE_LOCK(idx);
    failed = thread_unsafe_index_obj_replace(idx, index, new_data);
    OBJ_WRITE_UNLOCK(idx);
    return failed;
}

/**************************** Remove *****************************************/

PUBLIC int
index_obj_remove (index_obj_t *idx,
        void *data,
        void **data_removed,
        int shrink_threshold)
{
    int failed;
    
    OBJ_WRITE_LOCK(idx);
    failed = thread_unsafe_index_obj_remove(idx, data,
                data_removed, shrink_threshold);
    OBJ_WRITE_UNLOCK(idx);
    return failed;
}

/********************************* Reset *************************************/

PUBLIC void
index_obj_reset (index_obj_t *idx)
{
    OBJ_WRITE_LOCK(idx);
    idx->n = 0;
    (void) index_obj_resize(idx, INDEX_OBJECT_DEFAULT_SIZE);
    reset_stats(idx);
    OBJ_WRITE_UNLOCK(idx);
}

/**************************** Destroy ****************************************/

/*
 * This function first iterates thru all its elements and calls
 * the user specified data destruction callback function for each
 * non NULL entry in the index.  It then completely frees up the
 * storage associated with the index.
 */
PUBLIC void
index_obj_destroy (index_obj_t *idx,
        destruction_handler_t dh_fptr, void *extra_arg)
{
    int i;

    OBJ_WRITE_LOCK(idx);
    idx->should_not_be_modified = true;
    if (idx->elements) {
        if (dh_fptr) {
            for (i = 0; i < idx->n; i++)
                dh_fptr(idx->elements[i], extra_arg);
        }
        MEM_MONITOR_FREE(idx->elements);
    }
    OBJ_WRITE_UNLOCK(idx);
    LOCK_OBJ_DESTROY(idx);
    memset(idx, 0, sizeof(index_obj_t));
}

#ifdef __cplusplus
} // extern C
#endif 




