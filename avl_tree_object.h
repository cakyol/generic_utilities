
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

#ifndef __AVL_TREE_OBJECT_H__
#define __AVL_TREE_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <assert.h>

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"
#include "debug_framework.h"

typedef struct avl_node_s avl_node_t;

struct avl_node_s {

    /* used while iterating the tree */
    tinybool left_visited, right_visited;

    short balance;
    avl_node_t *parent;
    avl_node_t *left, *right;
    void *user_data;
};

typedef struct avl_tree_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    STATISTICS_VARIABLES;

    avl_node_t *root_node;
    object_comparer cmpf;
    bool should_not_be_modified;
    int n;

} avl_tree_t;

static inline int
avl_tree_size (avl_tree_t *tree)
{ return tree->n; }

extern int 
avl_tree_init (avl_tree_t *tree,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        object_comparer cmpf,
        mem_monitor_t *parent_mem_monitor);

extern void
avl_tree_debug_set_level (int level);

extern void
avl_tree_debug_set_module_name (char *name);

extern void
avl_tree_debug_set_reporting_function (debug_reporting_function drf);

extern int 
avl_tree_insert (avl_tree_t *tree,
        void *data_to_be_inserted,
        void **present_data,
        boolean overwrite_if_present);

extern int 
avl_tree_search (avl_tree_t *tree,
        void *data_to_be_searched,
        void **present_data);

extern int 
avl_tree_remove (avl_tree_t *tree,
        void *data_to_be_removed,
        void **data_actually_removed);

/*
 * Morris traverses the tree down from the specified 'root' parameter.
 * If 'root' is NULL, the entire tree will be traversed.
 * Here will be the parameters passed into the traversal function:
 *
 *  param0: tree
 *  param1: user data pointer of the node being traversed
 *  param2: p0
 *  param3: p1
 *  param4: p2
 *  param5: p3
 *
 *  Note that if 'tfn' returns NON zero, traversal will 'effectively'
 *  stop.  The reason I say 'effectively' is because the actual traversal
 *  of all the nodes will continue since in a morris traversal, we have
 *  to conclude the traversal till the end.  But the function will no
 *  longer be called.
 *
 *  The function return value should be 0 for continue and non
 *  zero (error code), to stop the traversal.
 */
extern int
avl_tree_morris_traverse (avl_tree_t *tree, avl_node_t *root,
        traverse_function_pointer tfn,
        void *p0, void *p1, void *p2, void *p3);

/*
 * This is a similar iterative way of traversing the tree,
 * always starting from the leftmost nodes.
 */
extern int
avl_tree_iterate (avl_tree_t *tree, avl_node_t *root,
        traverse_function_pointer tfn,
        void *p0, void *p1, void *p2, void *p3);

/*
 * Note that this destroys ONLY the contents of the object, NOT
 * the object itself, since it is not known whether this object
 * was statically or dynamically created.  It is up to the user
 * to free up the object itself (or not).  Note however that once
 * the object is destroyed, it is rendered unusable and MUST be
 * re-initialized if it needs to be reused again.  'dcbf' function
 * will be called for every user data with the extra argument
 * passed in.  The function can be specified as NULL.
 */
extern void 
avl_tree_destroy (avl_tree_t *tree,
        destruction_handler_t dcbf, void *extra_arg);

#ifdef __cplusplus
} // extern C
#endif 

#endif // __AVL_TREE_OBJECT_H__


