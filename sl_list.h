
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
** by having a special bit set in the node flags.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"
#include "debug_framework.h"

typedef struct list_node_s list_node_t;
typedef struct list_s list_t;

struct list_node_s {

    /* can be used for anything.  Check below */
    unsigned int flags;

    /* what list do I belong to */
    list_t *my_list;

    /* next node */
    list_node_t *next;

    /* user data opaque data */
    void *data;

};

/*
 * possible list node flags.
 * BIT 1 IS RESERVED, DO NOT CHANGE OR USE
 * UNDER ANY CIRCUMSTANCES.
 */
#define LIST_END_BIT        (1 << 0)

static inline
bool LIST_END (list_node_t *node)
{
    if (node->flags & LIST_END_BIT) {
        assert(node->data == NULL);
        assert(node->next == NULL);
        return true;
    }
    return false;
}

struct list_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    STATISTICS_VARIABLES;

    /* used for debugging */
    char name [REASONABLE_NAME_SIZE];

    list_node_t *head, *tail;

    /* how many nodes in list EXCLUDING the end (empty node) */
    int n;

    /* max nodes allowed in this list.  If 0, there is no limit */
    unsigned int n_max;

};

extern debug_module_block_t list_debug;

/*
 * if 'n_max is zero, the list has no artificially imposed limit.
 */
extern int
list_init (list_t *list, char *printable_name,
    bool make_it_thread_safe,
    bool enable_statistics,
    unsigned int n_max,
    mem_monitor_t *parent_mem_monitor);

extern int
list_prepend_data (list_t *list, void *data,
    list_node_t **ret_node);

extern int
list_append_data (list_t *list, void *data,
    list_node_t **ret_node);

extern list_node_t *
list_find_node (list_t *list, void *data);

extern int
list_remove_node (list_t *list, list_node_t *node);

extern int
list_remove_data (list_t *list, void *data);

extern void
list_destroy (list_t *list);

#ifdef __cplusplus
} // extern C
#endif

#endif /* __LIST_H__ */

