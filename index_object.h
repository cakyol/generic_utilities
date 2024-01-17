
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

/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
**
** @@@@@ INDEX OBJECT
**
** Generic insert/search/delete index.  Binary search based
** and hence extremely fast search.  However, slower in
** insertion & deletions.  But uses very little memory, only
** the size of a pointer per each element in the object.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __INDEX_OBJECT_H__
#define __INDEX_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"

typedef struct index_obj_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    STATISTICS_VARIABLES;

    boolean should_not_be_modified;
    object_comparer cmpf;
    int maximum_size;
    int expansion_size;
    int n;
    void **elements;

} index_obj_t;

/*
 * When the object is reset, it will have so many empty entries by default
 */
#define INDEX_OBJECT_DEFAULT_SIZE       8

/****************************** Initialize ***********************************
 *
 * This does not need much explanation, except maybe just the 'expansion_size'
 * field.  When an index object is initialized, its size is set to accept
 * 'maximum_size' entries.  If more than that number of entries are
 * needed, then the object self expands by 'expansion_size'.  If this value
 * is specified as 0, then expansion will not be allowed and the insertion
 * will fail.
 *
 * Function return value is errno or 0.
 */
extern int 
index_obj_init (index_obj_t *idx,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        object_comparer cmpf,
        int maximum_size,
        int expansion_size,
        mem_monitor_t *parent_mem_monitor);

/******************************** Insert *************************************
 *
 * Inserts 'data' into its appropriate place in the index.  If the data
 * was already in the index, also returns whatever was there in 'present_data'.
 * 'present_data' can be NULL if not needed.  If data was already there,
 * the existing data will be overwritten depending on how 'overwrite_if_present'
 * is specified.
 *
 * Function return value is errno or 0.
 */
extern int 
index_obj_insert (index_obj_t *idx,
        void *data,
        void **present_data,
        boolean overwrite_if_present);

/******************************** Search **************************************
 *
 * Search the data specified by 'data'.  Whatever is found, will be returned in
 * 'found'and 'index_found_at'. These can be set to NULL if not needed.
 *
 * Function return value is errno or 0.
 */
extern int 
index_obj_search (index_obj_t *idx,
        void *data,
        void **found, int *index_found_at);

/******************************** Replace **************************************
 *
 * This function replaces the user data pointer given at a certain index
 * with the newly specified user data, PROVIDED the indexing order is
 * maintained.
 *
 * It is a shortcut to changing an entry in the index instead of
 * first removing it and then insering it, which takes a much longer
 * time.
 *
 * *********** BUT ******************************
 *
 * YOU MUST BE EXTREMELY CAREFUL USING THIS TO MAKE SURE THAT THE NEW DATA
 * SPECIFIED DOES NOT CHANGE THE NATURAL ORDERING IN THE INDEX.
 *
 * USE AT YOUR DISCRETION.
 */
extern int
index_obj_replace (index_obj_t *idx,
        int index, void *new_data);

/********************************* Remove *************************************
 *
 * remove the data specified by 'data'.  What is removed is placed into
 * 'data_removed'.  This can be NULL if not needed.
 *
 * The 'shrink_threshold' value is used as a hint to whether the storage
 * used by the index should be reduced or not.  If it is 0 or -ve, no action
 * is taken.  However, if it is a positive number AND the number of empty
 * slots in the index is more than that value, the storage will be reduced
 * to the sum of the number of current elements in the index plus that
 * value.
 *
 * If for any reason the shrink fails (memory issue), the return value
 * of the function will not reflect that since the original remove function
 * is what determines the success but the index will not shrink but no 
 * harm will have been done.
 *
 * Function return value is errno or 0.
 */
extern int 
index_obj_remove (index_obj_t *idx,
        void *data,
        void **data_removed,
        int shrink_threshold);

/********************************* Reset **************************************
 *
 * This call resets the object back to as if it was completely empty with
 * the number of empty slots specified in INDEX_OBJECT_DEFAULT_SIZE.
 */
extern void
index_obj_reset (index_obj_t *idx);

/******************************** Destroy *************************************
 *
 * This calls destroys this index object and makes it un-usable.
 * Note that it destroys only the references to the objects, NOT
 * the object itself, since that is the caller's responsibility.
 * If the user wants to perform any operation on the actual user data
 * itself, it passes in a function pointer, (destruction handling
 * function pointer), which will be called for every item removed
 * off the object.  The parameters passed to that function will be
 * the user data pointer, and an extra arg the user may wish to
 * provide ('extra_arg').
 *
 * Note that the destruction handler function must not in any way
 * change anything on the index object itself.
 */
extern void
index_obj_destroy (index_obj_t *idx,
        destruction_handler_t dh_fptr, void *extra_arg);

#ifdef __cplusplus
} // extern C
#endif 

#endif // __INDEX_OBJECT_H__


