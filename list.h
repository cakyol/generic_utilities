
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
** Doubly linked list container object.
**
** This can be used as a lifo, fifo, queue, stk... basically
** any data structure which needs some kind of linked list.
** Since it is doubly linked, it is extremely fast to delete a
** node from it.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __DL_LIST_H__
#define __DL_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"

typedef struct list_node_s list_node_t;
typedef struct list_s list_t;

struct list_node_s {

    list_t *my_list;
    list_node_t *next, *prev;
    void *data;
};

struct list_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    STATISTICS_VARIABLES;

    list_node_t *head, *tail;

    /*
     * 0 means no limit. >0 means only so
     * many elements in the list are allowed
     */
    unsigned int n_max;

    /* how many nodes are in the list currently */
    int n;

};

/******************************************************************************
 * Initialize the list.  Return value is 0 for success
 * or a non zero errno.  
 *
 * If 'n_max' > 0, then the number of nodes in the list is limited
 * to that number.  Further insertions will not be allowed.  If
 * 'n_max' <= 0, there is no limit except the memory for insertions.
 */
extern int
list_init (list_t *list,
    bool make_it_thread_safe,
    bool enable_statistics,
    unsigned int n_max,
    mem_monitor_t *parent_mem_monitor);

/******************************************************************************
 * Add user data to the beginning of the list.
 * Return value is 0 for success or a non zero
 * errno value.  The node storing the data is returned
 * if the user wants to stash it somewhere to delete
 * it quickly later.
 */
extern int 
list_prepend_data (list_t *list, void *data,
    list_node_t **ret_node);

/******************************************************************************
 * Add user data to the end of the list.
 * Return value is 0 for success or a non zero
 * errno value.  The node storing the data is returned
 * if the user wants to stash it somewhere to delete
 * it quickly later.
 */
extern int
list_append_data (list_t *list, void *data,
    list_node_t **ret_node);

/******************************************************************************
 * Finds the data stored in the list and if found, returns
 * the node in which it is stored.  If not found, it returns
 * null.
 */
extern list_node_t *
list_find_data (list_t *list, void *data);

/******************************************************************************
 * Delete a node in the list, used when you
 * already know the node to be removed.
 */
extern int
list_remove_node (list_node_t *node);

/******************************************************************************
 * Delete the user data from the list.  If a comparison function
 * to compare user data pointers was defined at the init time of the
 * list, then that is used to find the node/data which is then
 * removed.
 */
extern int
list_remove_data (list_t *list, void *data);

/******************************************************************************
 * Destruction will be complete, list cannot be used until
 * it is re-initialized again properly.
 */
extern void
list_destroy (list_t *list);

#ifdef __cplusplus
} // extern C
#endif 

#endif // __DL_LIST_H__


