
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
** It ALWAYS is and WILL remain the sole property of Cihangir Metin Akyol.
**
** For proper indentation/viewing, regardless of which editor is being used,
** no tabs are used, ONLY spaces are used and the width of lines never
** exceed 80 characters.  This way, every text editor/terminal should
** display the code properly.  If modifying, please stick to this
** convention.
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
** Singly linked lifo used as a lifo.
**
** To keep things simple and extremely fast, insertions are ALWAYS
** performed to the head of the lifo and thats it (hence the name lifo_lifo).
**
** In this implementation, when a node is to be deleted, to speed up
** the execution, the NEXT node is copied over to this one and the NEXT
** node is deleted and voila, it is done.  No search to find the previous
** node is necessary.
**
** However, for this to be successful an always present 'end node' must be
** defined, which denotes the end of lifo, rather than simply NULL.
** This is implemented by having BOTH the 'next' and 'data' pointers of
** a node as NULL.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "lifo.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Always adds to the head
 */
static int
thread_unsafe_lifo_add_data (lifo_t *lifo, void *data,
    lifo_node_t **node)
{
    lifo_node_t *lnd;

    safe_pointer_set(node, NULL);
    if (lifo->n_max && (lifo->n >= lifo->n_max)) {
        insertion_failed(lifo);
        return ENOSPC;
    }
    lnd = MEM_MONITOR_ALLOC(lifo, sizeof(lifo_node_t));
    if (NULL == lnd) {
        insertion_failed(lifo);
        return ENOMEM;
    }
    lnd->data = data;
    lnd->next = lifo->head;
#ifdef EXTRA_CONSISTENCY_CHECKS
    lnd->my_lifo = lifo;
#endif /* EXTRA_CONSISTENCY_CHECKS */
    lifo->head = lnd;
    (lifo->n)++;
    safe_pointer_set(node, lnd);
    insertion_succeeded(lifo);

    return 0;
}

static int
thread_unsafe_lifo_search_data (lifo_t *lifo, void *data,
    lifo_node_t **node_found_at)
{
    lifo_node_t *n = lifo->head;

    safe_pointer_set(node_found_at, NULL);
    while (!END_NODE(n)) {
        if (n->data == data) {
                safe_pointer_set(node_found_at, n);
                search_succeeded(lifo);
                return 0;
        }
        n = n->next;
    }
    search_failed(lifo);
    return EEXIST;
}

static int
thread_unsafe_lifo_remove_node (lifo_t *lifo, lifo_node_t *lnd)
{
    lifo_node_t *to_be_freed;

#ifdef EXTRA_CONSISTENCY_CHECKS
    if (NULL == lnd) {
        deletion_failed(lifo);
        return EINVAL;
    }
    if (lnd->my_lifo != lifo) {
        deletion_failed(lifo);
        return EFAULT;
    }
#endif /* EXTRA_CONSISTENCY_CHECKS */

    /* can NEVER delete the end node */
    if (END_NODE(lnd)) {
        deletion_failed(lifo);
        return EINVAL;
    }

    /* copy the next node over to this one and delete the next one */
    to_be_freed = lnd->next;
    *lnd = *to_be_freed;
    MEM_MONITOR_FREE(to_be_freed);
    (lifo->n)--;
    deletion_succeeded(lifo);

    return 0;
}

PUBLIC int
lifo_init (lifo_t *lifo,
              bool make_it_thread_safe,
              bool enable_statistics,
              unsigned int n_max,
              mem_monitor_t *parent_mem_monitor)
{
    lifo_node_t *node;

    /* invalid values */
    if (NULL == lifo) return EINVAL;
    memset(lifo, 0, sizeof(lifo_t));

    /* Create end of lifo node, this is permanent */
    node = (lifo_node_t*) MEM_MONITOR_ALLOC(lifo, sizeof(lifo_node_t));
    if (NULL == node) return ENOMEM;
    node->next = NULL;
    node->data = NULL;

    MEM_MONITOR_SETUP(lifo);
    LOCK_SETUP(lifo);
    STATISTICS_SETUP(lifo);

    lifo->head = node;
    lifo->n_max = n_max;
    lifo->n = 0;

    return 0;
}

PUBLIC int
lifo_add_data (lifo_t *lifo, void *data,
    lifo_node_t **node)
{
    int failed;

    OBJ_WRITE_LOCK(lifo);
    failed = thread_unsafe_lifo_add_data(lifo, data, node);
    OBJ_WRITE_UNLOCK(lifo);

    return failed;
}

PUBLIC int
lifo_search_data (lifo_t *lifo, void *data,
    lifo_node_t **node)
{
    int failed;

    OBJ_READ_LOCK(lifo);
    failed = thread_unsafe_lifo_search_data(lifo, data, node);
    OBJ_READ_UNLOCK(lifo);

    return failed;
}

PUBLIC int
lifo_remove_node (lifo_t *lifo, lifo_node_t *node)
{
    int failed;

    OBJ_WRITE_LOCK(lifo);
    failed = thread_unsafe_lifo_remove_node(lifo, node);
    OBJ_WRITE_UNLOCK(lifo);

    return failed;
}

PUBLIC int
lifo_remove_data (lifo_t *lifo, void *data)
{
    int failed;
    lifo_node_t *node;

    /*
     * Although searching data does not need a write lock, since
     * we will eventually change the lifo, it is correct to perform
     * a write lock here.
     */
    OBJ_WRITE_LOCK(lifo);
    failed = thread_unsafe_lifo_search_data(lifo, data, &node);
    if (0 == failed) {
        failed = thread_unsafe_lifo_remove_node(lifo, node);
    }
    OBJ_WRITE_UNLOCK(lifo);

    return failed;
}

PUBLIC void
lifo_destroy (lifo_t *lifo)
{
    lifo_node_t *node, *del;

    OBJ_WRITE_LOCK(lifo);
    node = lifo->head;

    /* make sure the end node is also destroyed */
    while (node) {
        del = node;
        node = node->next;
        MEM_MONITOR_FREE(del);
    }

    OBJ_WRITE_UNLOCK(lifo);
    LOCK_OBJ_DESTROY(lifo);
    memset(lifo, 0, sizeof(lifo_t));
}

#ifdef __cplusplus
} // extern C
#endif


