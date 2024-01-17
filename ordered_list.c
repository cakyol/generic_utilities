
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

#include <stdlib.h>
#include <assert.h>
#include "ordered_list.h"

#define PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

static ordered_list_node_t *
ordered_list_new_node (ordered_list_t *listp, void *user_data)
{
    ordered_list_node_t *n;

    n = (ordered_list_node_t*) 
        MEM_MONITOR_ZALLOC(listp, sizeof(ordered_list_node_t));
    if (n) {
        n->list = listp;
        n->user_data = user_data;
        n->next = NULL;
    }
    return n;
}

/*
 * DONT MAKE THIS STATIC, it is used in scheduler.c
 *
 * data is always unconditionally added to the head of
 * the list, no duplicate checking is done.
 *
 * NOTE THAT THE USE OF THIS FUNCTION WILL COMPLETELY
 * DESTROY THE OBJECT ORDERING IN THE LIST WHICH WILL
 * CAUSE ALL SUBSEQUENT INSERT, SEARCH AND DELETE
 * OPERATIONS TO GO BERSERK.
 *
 * Despite this, sometimes it is necessary to maintain just
 * a simple list without any ordering.  If 'searching' in
 * the list is not needed, it is ok to use this.
 */
int
thread_unsafe_ordered_list_add_to_head (ordered_list_t *listp, void *user_data,
        ordered_list_node_t **node_added)
{
    ordered_list_node_t *node;

    node = ordered_list_new_node(listp, user_data);
    if (NULL == node) {
        safe_pointer_set(node_added, NULL);
        return ENOMEM;
    }

    /* insert to head */
    if (listp->head) node->next = listp->head;
    listp->head = node;
    listp->n++;

    /* return node to user */
    safe_pointer_set(node_added, node);

    /* done */
    return 0;
}

/*
 * add the new node/data based on ordering defined by 
 * the user specified comparison function.
 */
static int
thread_unsafe_ordered_list_add (ordered_list_t *listp, void *user_data,
        ordered_list_node_t **node_added)
{
    int result;
    ordered_list_node_t *cur, *prev;
    ordered_list_node_t *node;
    
    node = ordered_list_new_node(listp, user_data);
    if (NULL == node) {
        safe_pointer_set(node_added, NULL);
        return ENOMEM;
    }
    cur = listp->head;
    prev = NULL;
    while (not_endof_ordered_list(cur)) {
        result = listp->cmpf(cur->user_data, user_data);
        if (result >= 0) break;
        prev = cur;
        cur = cur->next;
    }
    if (prev) {
        node->next = prev->next;
        prev->next = node;
    } else {
        /* first element in the list */
        node->next = listp->head;
        listp->head = node;
    }
    listp->n++;
    safe_pointer_set(node_added, node);

    return 0;
}

static int
thread_unsafe_ordered_list_search (ordered_list_t *listp, void *searched_data,
        void **present_data,
        ordered_list_node_t **node_found,
        ordered_list_node_t **node_before)
{
    int result;
    ordered_list_node_t *cur, *prev;

    prev = NULL;
    cur = listp->head;
    while (not_endof_ordered_list(cur)) {

        result = listp->cmpf(cur->user_data, searched_data);

        /* an exact match */
        if (0 == result) {
            safe_pointer_set(present_data, cur->user_data);
            safe_pointer_set(node_found, cur);
            safe_pointer_set(node_before, prev);
            return 0;
        }

        /*
         * all the nodes past here must have values greater so
         * no point in continuing to search the rest of the list
         */
        if (result > 0) break;
        prev = cur;
        cur = cur->next;
    }

    /* not found */
    safe_pointer_set(present_data, NULL);
    safe_pointer_set(node_found, NULL);
    safe_pointer_set(node_before, prev);
    return ENODATA;
}

/*
 * This function will ensure that only one of the desired
 * entry gets inserted into the list.  0 will be returned
 * if successfull.  EEXIST will be returned if the exact same
 * entry is already in the list.  ENOMEM will be returned
 * if a new node cannot be allocated due to memory exhaustion.
 */
static int
thread_unsafe_ordered_list_add_once (ordered_list_t *listp, void *user_data, 
        void **present_data, ordered_list_node_t **node_found)
{
    int failed;
    ordered_list_node_t *node_added, *prev;

    failed =
        thread_unsafe_ordered_list_search(listp, user_data,
                present_data, node_found, &prev);

    /* found, already in list */
    if (0 == failed) return EEXIST;
    
    /* create the new node */
    node_added = ordered_list_new_node(listp, user_data);
    if (NULL == node_added) return ENOMEM;

    /* insert new node into its proper position */
    if (prev) {
        node_added->next = prev->next;
        prev->next = node_added;
    } else {
        node_added->next = listp->head;
        listp->head = node_added;
    }

    /* one more node */
    listp->n++;

    return 0;
}

/*
 * This is very clever, I got this off the internet.
 * Not sure about the authors name but I am quoting
 * it just to acknowledge his/her contribution.
 * The site from which the idea is taken is:
 * http://www.geeksforgeeks.org/in-a-linked-list-given-only-a-\
 *     pointer-to-a-node-to-be-deleted-in-a-singly-linked-list-\
 *     how-do-you-delete-it/
 *
 * Since we always have an "end of list" marker node 
 * (which is never NULL), this algorithm works even for 
 * deleting the last node in the list.
 */
int
thread_unsafe_ordered_list_node_delete (ordered_list_t *listp,
        ordered_list_node_t *node_tobe_deleted)
{
    ordered_list_node_t *to_free;

    /* should not delete end node, NEVER */
    if (endof_ordered_list(node_tobe_deleted))
        return EINVAL;

    /* if node is not part of list, no go */
    if (listp != node_tobe_deleted->list)
        return EINVAL;

    to_free = node_tobe_deleted->next;
    *node_tobe_deleted = *to_free;
    MEM_MONITOR_FREE(to_free);
    listp->n--;

    return 0;
}

static int
thread_unsafe_ordered_list_delete (ordered_list_t *listp,
        void *data_to_be_deleted, void **data_deleted)
{
    int failed;
    ordered_list_node_t *nd;

    failed = thread_unsafe_ordered_list_search
                (listp, data_to_be_deleted, data_deleted, &nd, NULL);
    if ((0 == failed) && nd) {
        return
            thread_unsafe_ordered_list_node_delete(listp, nd);
    }
    return failed;
}

/**************************** Initialize *************************************/

PUBLIC int
ordered_list_init (ordered_list_t *listp,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        object_comparer cmpf,
        mem_monitor_t *parent_mem_monitor)
{
    ordered_list_node_t *last_node;

    /* MUST have a compare function since list is ordered */
    if (NULL == cmpf) return EINVAL;

    MEM_MONITOR_SETUP(listp);
    LOCK_SETUP(listp);
    STATISTICS_SETUP(listp);

    last_node = (ordered_list_node_t*) 
                    MEM_MONITOR_ZALLOC(listp, sizeof(ordered_list_node_t));
    if (last_node) {
        last_node->list = listp;
        last_node->next = NULL;
        last_node->user_data = NULL;
        listp->head = last_node;
        listp->n = 0;
        listp->cmpf = cmpf;
        OBJ_WRITE_UNLOCK(listp);
        return 0;
    }
    return ENOMEM;
}

/**************************** Insert/add *************************************/

PUBLIC int
ordered_list_add (ordered_list_t *listp, void *user_data)
{
    int failed;

    OBJ_WRITE_LOCK(listp);
    failed = thread_unsafe_ordered_list_add(listp, user_data, NULL);
    OBJ_WRITE_UNLOCK(listp);
    return failed;
}

PUBLIC int
ordered_list_add_once (ordered_list_t *listp, void *user_data, 
        void **present_data)
{
    int failed;

    OBJ_WRITE_LOCK(listp);
    failed =
        thread_unsafe_ordered_list_add_once(listp, user_data,
            present_data, NULL);
    OBJ_WRITE_UNLOCK(listp);
    return failed;
}

/**************************** Search *****************************************/

PUBLIC int
ordered_list_search (ordered_list_t *listp, void *searched_data,
        void **present_data)
{
    int failed;

    OBJ_READ_LOCK(listp);
    failed = thread_unsafe_ordered_list_search
                (listp, searched_data, present_data, NULL, NULL);
    OBJ_READ_UNLOCK(listp);
    return failed;
}

/**************************** Remove/delete **********************************/ 

PUBLIC int
ordered_list_delete (ordered_list_t *listp, void *data_to_be_deleted,
        void **data_deleted)
{
    int failed;

    OBJ_WRITE_LOCK(listp);
    failed = thread_unsafe_ordered_list_delete(listp, data_to_be_deleted,
                data_deleted);
    OBJ_WRITE_UNLOCK(listp);
    return failed;
}

PUBLIC int
ordered_list_delete_node (ordered_list_t *listp,
        ordered_list_node_t *node_to_be_deleted)
{
    int failed;

    OBJ_WRITE_LOCK(listp);
    failed = thread_unsafe_ordered_list_node_delete(listp, node_to_be_deleted);
    OBJ_WRITE_UNLOCK(listp);
    return failed;
}

/**************************** Destroy ****************************************/ 

PUBLIC void
ordered_list_destroy (ordered_list_t *listp,
        destruction_handler_t dh_fptr, void *extra_arg)
{
    volatile void *user_data;

    OBJ_WRITE_LOCK(listp);
    while (not_endof_ordered_list(listp->head)) {

        /* stash the user data stored in the node */
        user_data = listp->head->user_data;

        /* destroy the node */
        thread_unsafe_ordered_list_node_delete(listp, listp->head);

        /*
         * call user function with the data, in case user
         * wants to do anything with it (most likely destroy)
         */
        if (dh_fptr) dh_fptr((void*) user_data, extra_arg);
    }

    /* free up the end of list marker */
    MEM_MONITOR_FREE(listp->head);
    listp->head = NULL;

    assert(0 == listp->n);
    OBJ_WRITE_UNLOCK(listp);
    LOCK_OBJ_DESTROY(listp);
    memset(listp, 0, sizeof(ordered_list_t));
}

#ifdef __cplusplus
} // extern C
#endif 




