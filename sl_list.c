
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
** Singly linked list.
**
** In this implementation, when a node is to be deleted, to speed up
** the execution, the NEXT node is copied over to this one and the NEXT
** node is deleted and voila, it is done.  No search to find the previous
** node is necessary.
**
** However, for this to be successful an 'end node' must be defined, which
** denotes the end of list, rather than simply NULL.  This is implemented
** by having BOTH the 'next' and 'data' pointers of a node as NULL.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

debug_module_block_t list_debug = {
    .lock = NULL,
    .level = ERROR_DEBUG_LEVEL,
    .module_name = "SINGLY_LINKED_LIST_MODULE",
    .drf = NULL
};

/*
 * Creates a node and performs all the basic operations
 * in preparation to add it to the list.
 */
static int
list_alloc_node (list_t *list, void *data, list_node_t **ret_node)
{
    int failed;
    list_node_t *node;

    node = MEM_MONITOR_ALLOC(list, sizeof(list_node_t));
    if (node) {
        node->flags = 0;
        node->next = NULL;
        node->data = data;
        node->my_list = list;
        failed = 0;
        *ret_node = node;
    } else {
        WARN(&list_debug,
            "list %s: MEM_MONITOR_ALLOC failed for %d bytes\n",
            list->name, sizeof(list_node_t));
        *ret_node = NULL;
        failed = ENOMEM;
    }
    return failed;
}

/*
 * To keep things very simple, always adds to the head.
 *
 * Note that the node in which the data is stored is returned to
 * the user in 'ret_node'.  This is so that the user may cache this
 * somewhere in the object which may then be used if deletion of the
 * object from the list is needed.  The node can be accessed very
 * quickly for such a removal.  If the user is not interested, a NULL
 * may be passed as a valid parameter for ret_node.
 */
static int
thread_unsafe_list_prepend_node (list_t *list, void *data,
    list_node_t **ret_node)
{
    int failed;
    list_node_t *node;

    safe_pointer_set(ret_node, NULL);

    /* is list limit reached ? */
    if (list->n_max &&
        ((unsigned int) list->n >= list->n_max)) {
            WARN(&list_debug, "list %s: list full for prepend (limit %u)\n",
                list->name, list->n_max);
            return ENOSPC;
    }

    failed = list_alloc_node(list, data, &node);
    if (failed) {
        insertion_failed(list);
        return failed;
    }
    
    node->next = list->head;
    list->head = node;
    if (NULL == list->tail) {
        assert(list->n == 0);
        list->tail = node;
    }
    
    list->n++;
    safe_pointer_set(ret_node, node);
    insertion_succeeded(list);

    return 0;
}

static int
thread_unsafe_list_append_node (list_t *list, void *data,
    list_node_t **ret_node)
{
    int failed;
    list_node_t *node;

    safe_pointer_set(ret_node, NULL);

    /* is list limit reached ? */
    if (list->n_max &&
        ((unsigned int) list->n >= list->n_max)) {
            WARN(&list_debug, "list %s: list full for append (limit %u)\n",
                list->name, list->n_max);
            return ENOSPC;
    }
    failed = list_alloc_node(list, data, &node);
    if (failed) {
        insertion_failed(list);
        return failed;
    }
}

/*
 * Return the node just one before 'node'.
 * Return NULL if the only node in the list.
 * Should be called ONLY when you are SURE 
 * that there is more than one node at least
 * in the list.
 */
static list_node_t *
list_get_previous_node (list_t *list, list_node_t *node)
{
    list_node_t *n;

    assert(list == node->my_list);
    assert(list->n > 1);
    n = list->head;
    while (true) {
        if (LIST_END(n)) return NULL;
        if (n->next == node) return n;
        n = n->next;
    }
    return NULL;
}

static int
thread_unsafe_list_remove_node (list_t *list, list_node_t *node)
{
    list_node_t *to_be_freed;

    /* this can never be -ve */
    assert(list->n >= 0);

    if (list->n <= 0) {
        deletion_failed(list);
        WARN(&list_debug, "list %s: no elements in list\n", list->name);
        return ENOENT;
    }
    if (LIST_END(node)) {
        deletion_failed(list);
        return EFAULT;
    }
    if (node->my_list != list) {
        deletion_failed(list);
        return EFAULT;
    }

    to_be_freed = node->next;
    *node = *to_be_freed;
    MEM_MONITOR_FREE(to_be_freed);

    list->n--;
    deletion_succeeded(list);

    return 0;
}

PUBLIC int
list_init (list_t *list, char *printable_name,
              bool make_it_thread_safe,
              bool enable_statistics,
              unsigned int n_max,
              mem_monitor_t *parent_mem_monitor)
{
    int failed;
    list_node_t *node;

    /* invalid values */
    if (NULL == list) return EINVAL;

    MEM_MONITOR_SETUP(list);
    LOCK_SETUP(list);
    STATISTICS_SETUP(list);

    /* create end of list node */
    failed = list_alloc_node(list, NULL, &node);
    if (failed) return failed;
    node->flags |= LIST_END_BIT;

    /* assign list name */
    if (printable_name) {
        strncpy(&list->name[0], printable_name, REASONABLE_NAME_SIZE);
    } else {
        snprintf(&list->name[0], REASONABLE_NAME_SIZE, "0x%p", list);
    }
    list->name[REASONABLE_NAME_SIZE - 1] = 0;

    list->head = node;
    list->tail = NULL;
    list->n_max = n_max;
    list->n = 0;

    return 0;
}

PUBLIC int
list_prepend_data (list_t *list, void *data, list_node_t **ret_node)
{
    int failed;

    OBJ_WRITE_LOCK(list);
    failed = thread_unsafe_list_prepend_node(list, data, ret_node);
    OBJ_WRITE_UNLOCK(list);
    return failed;
}

PUBLIC list_node_t *
list_find_node (list_t *list, void *data)
{
    list_node_t *node, *found = NULL;

    OBJ_WRITE_LOCK(list);
    node = list->head;
    while (!LIST_END(node)) {
        if (node->data == data) {
            found = node;
            break;
        }
        node = node->next;
    }
    OBJ_WRITE_UNLOCK(list);
    return found;
}

PUBLIC int
list_remove_node (list_t *list, list_node_t *node)
{
    int failed;

    OBJ_WRITE_LOCK(list);
    failed = thread_unsafe_list_remove_node(list, node);
    OBJ_WRITE_UNLOCK(list);
    return failed;
}

PUBLIC int
list_remove_data (list_t *list, void *data)
{
    int failed = ENOENT;
    list_node_t *node;

    OBJ_WRITE_LOCK(list);
    node = list_find_node(list, data);
    if (node) {
        failed = list_remove_node(list, node);
    }
    OBJ_WRITE_UNLOCK(list);
    return failed;
}

PUBLIC void
list_destroy (list_t *list)
{
    list_node_t *n, *d;

    OBJ_WRITE_LOCK(list);
    n = list->head;
    while (n) {
        d = n;
        n = n->next;
        MEM_MONITOR_FREE(d);
    }
    OBJ_WRITE_UNLOCK(list);
    LOCK_OBJ_DESTROY(list);
    memset(list, 0, sizeof(list_t));
}

#ifdef __cplusplus
} // extern C
#endif


