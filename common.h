
/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
**
** Author: Cihangir Metin Akyol, gee.akyol@gmail.com, gee_akyol@yahoo.com
** Copyright: Cihangir Metin Akyol, April 2014 -> ....
**
** All this code has been personally developed by and belongs to 
** Mr. Cihangir Metin Akyol.  It has been developed in his own 
** personal time using his own personal resources.  Therefore,
** it is NOT owned by any establishment, group, company or 
** consortium.  It is the sole property and work of the named
** individual.
**
** It CAN be used by ANYONE or ANY company for ANY purpose as long 
** as ownership and/or patent claims are NOT made to it by ANYONE
** or ANY ENTITY.
**
** It ALWAYS is and WILL remain the property of Cihangir Metin Akyol.
**
** For proper indentation viewing, no tabs are used.  This way, every
** text editor should display the code properly.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

/*
 * This file is a collection of the most common types/definitions
 * which is used by almost all the generic utilities.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PUBLIC
#define PUBLIC
#endif /* PUBLIC */

/*
 * This supresses the compiler error messages for unused variables,
 * if the error detections are fully turned on during compilations
 * which is a nuisance but luckily can be turned off with this.
 */
#ifndef SUPPRESS_UNUSED_VARIABLE_COMPILER_WARNING
#define SUPPRESS_UNUSED_VARIABLE_COMPILER_WARNING(x)    ((void)(x))
#endif /* SUPPRESS_UNUSED_VARIABLE_COMPILER_WARNING */

#ifndef NULL
#define NULL        0 
#endif /* NULL */

#ifndef null
#define null        0
#endif /* null */

#ifndef TRUE
#define TRUE        1
#endif /* TRUE */

#ifndef true
#define true        1
#endif /* true */

#ifndef FALSE 
#define FALSE       0
#endif /* FALSE */

#ifndef false
#define false       0
#endif /* false */

/*
 * unsigned byte
 */
#ifndef TYPEDEF_BYTE
typedef unsigned char byte;
#define TYPEDEF_BYTE
#endif /* TYPEDEF_BYTE */

#ifndef TYPEDEF_BOOL
typedef char tinybool;
typedef short sbool;
typedef int bool;
typedef int boolean;
#define TYPEDEF_BOOL
#endif /* TYPEDEF_BOOL */

#define REASONABLE_NAME_SIZE    32

/*************************************************************************/

/*
 * Handles overlapping copies
 */
static inline void
copy_bytes (void *src, void *dst, int count)
{
    byte *bsrc = (byte*) src;
    byte *bdst = (byte*) dst;

    if (dst < src) {
        while (count-- > 0) *bdst++ = *bsrc++;
    } else {
        bsrc += count;
        bdst += count;
        while (count-- > 0) *(--bdst) = *(--bsrc);
    }
}

/*
 * Handles overlapping copies.
 *
 * This function copies the desired number of continuous array entries
 * where each element of the array is assumed to be of 'element_size' bytes
 * each, from a source array index to a destination array index.  It
 * is assumed that the arrays hold such elements in each of their slots.
 * The array start addresses, the indexes and the size of each array
 * element in bytes is given as parameters.
 */
static inline void
copy_array_blocks (void *src_array_addr_start, int src_index,
    void *dst_array_addr_start, int dst_index,
    int how_many_chunks, int element_size) 
{
    byte *src_bytes = (byte*) src_array_addr_start;
    byte *dst_bytes = (byte*) dst_array_addr_start;

    memmove(&dst_bytes[dst_index * element_size],
        &src_bytes[src_index * element_size],
        how_many_chunks * element_size);
}

/*
 * Handles overlapping copies.
 *
 * A special case of 'copy_array_blocks' above where each array element is
 * actually a pointer.  This makes it much faster to copy pointers.
 */
static inline void
copy_pointer_blocks (void **src, void **dst, int count)
{
    if (dst < src)
        while (count-- > 0) *dst++ = *src++;
    else {
        src += count;
        dst += count;
        while (count-- > 0) *(--dst) = *(--src);
    }
}

/*************************************************************************/

#ifdef INCLUDE_STATISTICS

    /*
     * Typical statistics used for most insert/search/delete
     * type of data structures.  Mostly usable for debugging.
     * Not every field is used in all objects.  For example,
     * in the queue object, "search*' counters are not used,
     * since they do not make sense.  They will stay as 0.
     */
    typedef struct statistics_block_s {

        unsigned long long int insertion_successes;
        unsigned long long int insertion_failures;

        unsigned long long int search_successes;
        unsigned long long int search_failures;

        unsigned long long int deletion_successes;
        unsigned long long int deletion_failures;

    } statistics_block_t;

    #define STATISTICS_VARIABLES \
        statistics_block_t stats, *stats_p

    #define STATISTICS_SETUP(objp) \
        if (enable_statistics) { \
            objp->stats_p = &(objp->stats); \
        } else { \
            objp->stats_p = NULL; \
        }

    /*
     * update/increment values
     */
    #define insertion_succeeded(objp) \
        if (objp->stats_p) objp->stats_p->insertion_successes++
    #define insertion_failed(objp) \
        if (objp->stats_p) objp->stats_p->insertion_failures++
    #define insertion_stats_update(objp, failed) \
        if (objp->stats_p) { \
            if (failed) { \
                objp->stats_p->insertion_failures++; \
            } else { \
                objp->stats_p->insertion_successes++; \
            } \
        }

    #define search_succeeded(objp) \
        if (objp->stats_p) objp->stats_p->search_successes++
    #define search_failed(objp) \
        if (objp->stats_p) objp->stats_p->search_failures++
    #define search_stats_update(objp, failed) \
        if (objp->stats_p) { \
            if (failed) { \
                objp->stats_p->search_failures++; \
            } else { \
                objp->stats_p->search_successes++; \
            } \
        }

    #define deletion_succeeded(objp) \
        if (objp->stats_p) objp->stats_p->deletion_successes++
    #define deletion_failed(objp) \
        if (objp->stats_p) objp->stats_p->deletion_failures++
    #define deletion_stats_update(objp, failed) \
        if (objp->stats_p) { \
            if (failed) { \
                objp->stats_p->deletion_failures++; \
            } else { \
                objp->stats_p->deletion_successes++; \
            } \
        }

    /*
     * read/get values
     */
    #define get_insertion_successes(objp) \
        ((objp && objp->stats_p) ? objp->stats_p->insertion_successes : 0)
    #define get_insertion_failures(objp) \
        ((objp && objp->stats_p) ? objp->stats_p->insertion_failures : 0)
    #define get_search_successes(objp) \
        ((objp && objp->stats_p) ? objp->stats_p->search_successes : 0)
    #define get_search_failures(objp) \
        ((objp && objp->stats_p) ? objp->stats_p->search_failures : 0)
    #define get_deletion_successes(objp) \
        ((objp && objp->stats_p) ? objp->stats_p->deletion_successes : 0)
    #define get_deletion_failures(objp) \
        ((objp && objp->stats_p) ? objp->stats_p->deletion_failures : 0)

    /*
     * reset all stats counters
     */
    #define reset_stats(objp) \
        memset(&(objp->stats), 0, sizeof(statistics_block_t))

#else /* !INCLUDE_STATISTICS */

    typedef struct statistics_block_s {
    } statistics_block_t;

    #define STATISTICS_VARIABLES
    #define STATISTICS_SETUP(objp)
    
    #define insertion_succeeded(objp)
    #define insertion_failed(objp)
    #define insertion_stats_update(objp, failure)
    
    #define search_succeeded(objp)
    #define search_failed(objp)
    #define search_stats_update(objp, failure)

    #define deletion_succeeded(objp)
    #define deletion_failed(objp)
    #define deletion_stats_update(objp, failure)
    
    /*
     * read/get values
     */
    #define get_insertion_successes(objp)   0
    #define get_insertion_failures(objp)    0
    #define get_search_successes(objp)      0
    #define get_search_failures(objp)       0
    #define get_deletion_successes(objp)    0
    #define get_deletion_failures(objp)     0

    #define reset_stats(objp)
    
#endif /* INCLUDE_STATISTICS */

/*************************************************************************/

typedef int
(*one_parameter_function_pointer)(void *arg1);

typedef int
(*two_parameter_function_pointer)(void *arg1, void *arg2);

typedef int
(*three_parameter_function_pointer)
    (void *arg1, void *arg2, void *arg3);

typedef int
(*four_parameter_function_pointer)
    (void *arg1, void *arg2, void *arg3, void *arg4);

typedef int
(*five_parameter_function_pointer)
    (void *arg1, void *arg2, void *arg3, void *arg4,
        void *arg5);

typedef int
(*six_parameter_function_pointer)
    (void *arg1, void *arg2, void *arg3, void *arg4,
        void *arg5, void *arg6);

typedef int (*seven_parameter_function_pointer)
    (void *arg1, void *arg2, void *arg3, void *arg4,
        void *arg5, void *arg6, void *arg7);

typedef one_parameter_function_pointer simple_function_pointer;

typedef two_parameter_function_pointer object_comparer;

typedef seven_parameter_function_pointer traverse_function_pointer;

/*
 * This is a function prototype which will be called when an object
 * is being destroyed.  It can be any object being destroyed.
 * It can be an avl tree, index object, list object etc.  The idea
 * is, as the object itself is being destroyed (its nodes being freed
 * back to storage), this is called with the actual 'user_data' stored on 
 * that node, in case user wants to also destroy/free their own
 * data itself.  It will be called one node at a time as the destruction
 * happens.  When this is called, the user MUST be aware that their data
 * has already been REMOVED from whatever object it was a part of, and
 * pretty much 'dangling' by itself.  Typically user will want to 
 * free up his/her object but this may not always be the case.
 * The FIRST parameter passed into this function is the user data itself
 * and the second parameter is whatever the user supplied at the time of
 * the destruction call.  It goes without saying that this function
 * should NOT make any object calls.
 */
typedef void (*destruction_handler_t)(void *user_data, void *extra_arg);

/*
 * Sets a pointer to an integer value.  This can be directly done when
 * a pointer is the same size as an integer but is usually complained 
 * about by most compilers otherwise.  This little trick solves that.
 */
static inline void*
integer2pointer (long long int value)
{
    byte *ptr = NULL;
    ptr += value;
    return ptr;
}

static inline long long int
pointer2integer (void *ptr)
{
    byte *zero = NULL;
    return ((byte*) ptr) - zero;
}

#define pointer_from_integer(i)         integer2pointer((long long int) (i))
#define integer_from_pointer(p)         pointer2integer((void*) (p))
#define safe_pointer_set(ptr, value)    if (ptr) *(ptr) = (value)

#ifdef __cplusplus
} /* extern C */
#endif 

#endif /* __COMMON_H__ */



