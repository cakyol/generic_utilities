
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
** doubly linked list
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

static list_node_t *
list_create_new_node (list_t *list, void *data,
    int *err)
{
    list_node_t *node;

    if (list->n_max && (list->n >= (int) list->n_max)) {
        *err = ENOSPC;
        return NULL;
    }
    node = MEM_MONITOR_ALLOC(list, sizeof(list_node_t));
    if (node) {
        node->my_list = list;
        node->next = node->prev = NULL;
        node->data = data;
        *err = 0;
    } else {
        *err = ENOMEM;
    }
    return node;
}

static list_node_t *
thread_unsafe_list_prepend_data (list_t *list, void *data,
    int *err)
{
    list_node_t *node;

    node = list_create_new_node(list, data, err);
    if (NULL == node) {
        insertion_failed(list);
        return NULL;
    }
    
    if (list->n <= 0) {
        list->head = list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->n++;

    insertion_succeeded(list);
    return node;
}

static list_node_t *
thread_unsafe_list_append_data (list_t *list, void *data,
    int *err)
{
    list_node_t *node;

    node = list_create_new_node(list, data, err);
    if (NULL == node) {
        insertion_failed(list);
        return NULL;
    }

    if (list->n <= 0) {
        list->head = list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    insertion_succeeded(list);
    list->n++;

    return node;
}

static list_node_t *
thread_unsafe_list_find_data_node (list_t *list, void *data)
{
    list_node_t *node;

    node = list->head;
    while (node) {
        if (data == node->data) return node;
        node = node->next;
    }
    return NULL;
}

static int
thread_unsafe_list_remove_node (list_t *list, list_node_t *node)
{
    if (node->next == NULL) {
        if (node->prev == NULL) {
            list->head = list->tail = NULL;
        } else {
            node->prev->next = NULL;
            list->tail = node->prev;
        }
    } else {
        if (node->prev == NULL) {
            list->head = node->next;
            node->next->prev = NULL;
        } else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }
    }
    MEM_MONITOR_FREE(node);
    list->n--;
    assert(list->n >= 0);

    deletion_succeeded(list);
    return 0;
}

/************************** Public functions **************************/

PUBLIC int
list_init (list_t *list,
    bool make_it_thread_safe,
    bool enable_statistics,
    unsigned int n_max,
    mem_monitor_t *parent_mem_monitor)
{
    MEM_MONITOR_SETUP(list);
    LOCK_SETUP(list);
    STATISTICS_SETUP(list);

    list->head = list->tail = NULL;
    list->n_max = n_max;
    list->n = 0;

    return 0;
}

PUBLIC int
list_prepend_data (list_t *list, void *data,
    list_node_t **ret_node)
{
    int failed;

    OBJ_WRITE_LOCK(list);
    *ret_node = thread_unsafe_list_prepend_data(list, data, &failed);
    OBJ_WRITE_UNLOCK(list);

    return failed;
}

PUBLIC int
list_append_data (list_t *list, void *data,
    list_node_t **ret_node)
{
    int failed;
    list_node_t *node;

    OBJ_WRITE_LOCK(list);
    node = thread_unsafe_list_append_data(list, data, &failed);
    safe_pointer_set(ret_node, node);
    OBJ_WRITE_UNLOCK(list);

    return failed;
}

PUBLIC list_node_t *
list_find_data_node (list_t *list, void *data)
{
    list_node_t *node;

    OBJ_READ_LOCK(list);
    node = thread_unsafe_list_find_data_node(list, data);
    search_stats_update(list, node);
    OBJ_READ_UNLOCK(list);

    return node;
}

PUBLIC int
list_remove_node (list_node_t *node)
{
    int failed;
    list_t *list;

    if (NULL == node) return EINVAL;
    list = node->my_list;
    OBJ_WRITE_LOCK(list);
    failed = thread_unsafe_list_remove_node(list, node);
    OBJ_WRITE_UNLOCK(list);

    return failed;
}

PUBLIC int
list_remove_data (list_t *list, void *data)
{
    list_node_t *node;
    int failed;

    OBJ_WRITE_LOCK(list);
    node = thread_unsafe_list_find_data_node(list, data);
    if (node) {
        thread_unsafe_list_remove_node(list, node);
        failed = 0;
    } else {
        failed = ENODATA;
    }
    deletion_stats_update(list, failed);
    OBJ_WRITE_UNLOCK(list);

    return failed;
}

void
list_destroy (list_t *list)
{
    list_node_t *node, *next_node;

    OBJ_WRITE_LOCK(list);
    node = list->head;
    while (node) {
        next_node = node->next;
        MEM_MONITOR_FREE(node);
        node = next_node;
    }
    OBJ_WRITE_UNLOCK(list);
    memset(list, 0, sizeof(list_t));
    lock_obj_destroy(list->lock);
}

#ifdef __cplusplus
} // extern C
#endif 




