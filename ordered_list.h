
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
** Ordered singly linked list object, with very fast deletion provided you
** know the node that you want to delete.  You do NOT have to know
** the 'previous' node.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __ORDERED_LIST_H__
#define __ORDERED_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"

typedef struct ordered_list_s ordered_list_t;
typedef struct ordered_list_node_s ordered_list_node_t;

/*
 * Users should be aware that the list is terminated with an
 * ACTUAL node whose next pointer AND the data pointer is NULL.
 * An 'end of list' marker node is needed to implement very fast
 * deletion from the list.
 *
 * Therefore, when iterating thru a list, take into account
 * the fact that the LAST node is ONLY a marker and NOT
 * real data.
 */
struct ordered_list_node_s {

    /* what list I belong to */
    ordered_list_t *list;

    /* next node */
    ordered_list_node_t *next;

    /* user opaque data */
    void *user_data;
};

/*
 * singly linked list object.
 * comparison function is used to test ranking/ordering of list elements.
 */
struct ordered_list_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    STATISTICS_VARIABLES;

    /* start of list */
    ordered_list_node_t *head;

    /*
     * comparison function used for ordering the list.
     * MUST always be specified at init time.
     */
    object_comparer cmpf;

    /* number of elements in the list */
    int n;

};

/*
 * We maintain an "end node" always in the list.  This is represented
 * by setting its 'next' AND data pointers both to the value of NULL.
 *
 * The reason we need an "end node" representation is so that we
 * can delete a node VERY quickly.  What we do to delete a node
 * is actually copy the contents of the NEXT node onto THIS one and delete
 * the NEXT node.  This will simulate the effect of having deleted THIS
 * node, which is exactly what is wanted.  And since we always have the 
 * "next" node pointer available even in the LAST valid node, this
 * is very easy.  However, this would fail if the last node was being
 * deleted since there would be nothing past the last node.  To alleviate
 * this we ALWAYS maintain an "end node".  The "end node" holds NO data
 * and is ONLY a marker.
 */

static inline int
endof_ordered_list (ordered_list_node_t *olnp)
{
    return
        (NULL == olnp) || 
        ((NULL == olnp->next) && (NULL == olnp->user_data));
}

static inline int
not_endof_ordered_list (ordered_list_node_t *olnp)
{
    return
        olnp && (NULL != olnp->next) && (NULL != olnp->user_data);
}

/*
 * initializes the linked list object
 */
extern int
ordered_list_init (ordered_list_t *listp,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        object_comparer cmpf,
        mem_monitor_t *parent_mem_monitor);

/*
 * adds a node containing the specified data to the list.
 * ALWAYS added, regardless of whether the data is already 
 * in the list or not.  So, multiple copies WILL be added
 * if not controlled.  Addition is is done at a point in
 * the list determined by the ordering specified in the
 * comparison function. 
 */
extern int
ordered_list_add (ordered_list_t *listp, void *user_data);

/*
 * same as adding data but the data is added only once.
 * If it is already there, it will not be added again and
 * 'present_data' will be updated with what is already
 * found in the list.  If data was not already in the list,
 * 'present_data' will be set to NULL.  This mechanism
 * always gives the user the knowledge of whether the
 * data being inserted was in the list in the first place.
 * If 'present_data' is not needed, NULL can be passed in.
 * The return value will be 0 if the addition succeeded.
 * Errors will be returned for any other condition,
 * including if the data was already in the list (EEXIST).
 */
extern int
ordered_list_add_once (ordered_list_t *listp, void *user_data, 
        void **present_data);

/*
 * searches the first occurence of the matching data.
 * This is where the compare function is applied to find
 * out whether a match has occured.
 * Function return value will be 0 if found and 'present_data'
 * will be set to the found data.  If not found,
 * non 0 will be returned and 'present_data' will be set 
 * to NULL.  If 'present_data' is not needed, it can be
 * passed in as NULL.
 */
extern int
ordered_list_search (ordered_list_t *listp, void *searched_data, 
        void **present_data);

/*
 * Removes the entry in the list matching user data specified
 * by 'to_be_deleted'.  If data was in the list, it will be
 * removed, 0 will be returned and 'data_deleted' will be set to
 * what was found in the list.  Otherwise, non 0 will be returned
 * and 'data_deleted' will be set to NULL.  If 'data_deleted' is not
 * needed, it can be passed in as NULL.
 */
extern int 
ordered_list_delete (ordered_list_t *listp, void *to_be_deleted,
        void **data_deleted);

/*
 * Note that this destroys ONLY the contents of the object, NOT
 * the object itself, since it is not known whether this object
 * was statically or dynamically created.  It is up to the user
 * to free up the object itself (or not).  Note however that once
 * the object is destroyed, it is rendered unusable and MUST be
 * re-initialized if it needs to be reused again.
 */
extern void
ordered_list_destroy (ordered_list_t *listp,
        destruction_handler_t dh_fptr, void *extra_arg);

/*
 * Some convenient macros to iterate thru the list one node at a time.
 */

#define FOR_ALL_ORDEREDLIST_ELEMENTS(listp, objp) \
        for (ordered_list_node_t *__n__ = (listp)->head; \
             not_endof_ordered_list(__n__) && \
             (objp = (__typeof__(objp))(__n__->user_data)); \
             __n__ = __n__->next)

/*
 * Used for self depleting lists, ie at every iteration, the
 * head element is EXPECTED to be removed by the iteration body.
 * If that is not done, infinite loops WILL result.
 */
#define WHILE_ORDEREDLIST_NOT_EMPTY(listp, objp) \
        for (ordered_list_node_t *__n__ = (listp)->head; \
             not_endof_ordered_list(__n__) && \
             (objp = (__typeof__(objp))(__n__->user_data)); \
             __n__ = listp->head)

#ifdef __cplusplus
} // extern C
#endif 

#endif // __ORDERED_LIST_H__






