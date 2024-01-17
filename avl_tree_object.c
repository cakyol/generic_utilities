
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

#include "avl_tree_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * default values for avl tree object debugging
 */
static debug_module_block_t avl_debug = {

    .lock = NULL,
    .level = ERROR_DEBUG_LEVEL,
    .module_name = "AVL_TREE_MODULE",
    .drf = NULL
};

static inline avl_node_t *
get_first (avl_node_t *node)
{
    while (node->left) node = node->left;
    return node;
}

static inline void 
set_child (avl_node_t *child, avl_node_t *parent, 
    int left)
{
    if (left)
        parent->left = child;
    else
        parent->right = child;
}

#if 0

static inline avl_node_t *
get_last (avl_node_t *node)
{
    while (node->right) node = node->right;
    return node;
}

static inline avl_node_t *
avl_tree_next (avl_node_t *node)
{
    avl_node_t *parent;

    if (node->right)
        return 
            get_first(node->right);

    while ((parent = node->parent) && 
        (parent->right == node))
            node = parent;

    return parent;
}

static inline avl_node_t *
avl_tree_prev (avl_node_t *node)
{
    avl_node_t *parent;

    if (node->left)
        return 
            get_last(node->left);

    while ((parent = node->parent) && 
        (parent->left == node))
            node = parent;
        
    return parent;
}

#endif // 0

static inline void 
rotate_left (avl_node_t *node, avl_tree_t *tree)
{
    avl_node_t *p = node;
    avl_node_t *q = node->right;
    avl_node_t *parent = p->parent;

    if (parent) {
        if (parent->left == p)
            parent->left = q;
        else
            parent->right = q;
    } else
        tree->root_node = q;
    q->parent = parent;
    p->parent = q;
    p->right = q->left;
    if (p->right)
        p->right->parent = p;
    q->left = p;
}

static inline void 
rotate_right (avl_node_t *node, avl_tree_t *tree)
{
    avl_node_t *p = node;
    avl_node_t *q = node->left;
    avl_node_t *parent = p->parent;

    if (parent) {
        if (parent->left == p)
            parent->left = q;
        else
            parent->right = q;
    } else
        tree->root_node = q;
    q->parent = parent;
    p->parent = q;
    p->left = q->right;
    if (p->left)
        p->left->parent = p;
    q->right = p;
}

static avl_node_t *
avl_lookup_engine (avl_tree_t *tree,
        void *searched,
        avl_node_t **pparent, avl_node_t **unbalanced, 
        int *is_left)
{
    avl_node_t *node = tree->root_node;
    int res = 0;

    *pparent = NULL;
    *unbalanced = node;
    *is_left = 0;

    while (node) {
        if (node->balance) *unbalanced = node;
        res = (tree->cmpf)(searched, node->user_data);
        if (res == 0) return node;
        *pparent = node;
        if ((*is_left = (res < 0))) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return NULL;
}

static inline void
free_avl_node (avl_tree_t *tree, avl_node_t *node)
{
    MEM_MONITOR_FREE(node);
    tree->n--;
}

static inline avl_node_t *
new_avl_node (avl_tree_t *tree, void *user_data)
{
    avl_node_t *node = MEM_MONITOR_ALLOC(tree, sizeof(avl_node_t));

    if (node) {
        node->left_visited = node->right_visited = false;
        node->parent = node->left = node->right = NULL;
        node->balance = 0;
        node->user_data = user_data;
        tree->n++;
    }
    return node;
}

static int 
thread_unsafe_avl_tree_insert (avl_tree_t *tree,
    void *data,
    void **present_data,
    boolean overwrite_if_present)
{
    avl_node_t *found, *parent, *unbalanced, *node;
    int is_left;

    /* assume the entry is not present initially */
    safe_pointer_set(present_data, NULL);

    /*
     * some kind of traversal is already happening on the tree,
     * so any kind of access which may change the tree is
     * not allowed
     */
    if (tree->should_not_be_modified) {
        insertion_failed(tree);
        return EBUSY;
    }

    found = avl_lookup_engine(tree, data,
                &parent, &unbalanced, &is_left);

    if (found) {
        safe_pointer_set(present_data, found->user_data);
        if (overwrite_if_present) {
            found->user_data = data;
            insertion_succeeded(tree);
        }
        return 0;
    }

    /* get a new node */
    node = new_avl_node(tree, data);
    if (NULL == node) {
        insertion_failed(tree);
        return ENOMEM;
    }

    if (!parent) {
        tree->root_node = node;
        return 0;
    }

    node->parent = parent;
    set_child(node, parent, is_left);
    for (;;) {
        if (parent->left == node)
            (parent->balance)--;
        else
            (parent->balance)++;
        if (parent == unbalanced)
            break;
        node = parent;
        parent = parent->parent;
    }

    switch (unbalanced->balance) {
    case  1: 
    case -1:
    case 0:
        break;
        
    case 2: 
        {
            avl_node_t *right = unbalanced->right;

            if (right->balance == 1) {
                unbalanced->balance = 0;
                right->balance = 0;
            } else {
                switch (right->left->balance) {
                case 1:
                    unbalanced->balance = -1;
                    right->balance = 0;
                    break;
                case 0:
                    unbalanced->balance = 0;
                    right->balance = 0;
                    break;
                case -1:
                    unbalanced->balance = 0;
                    right->balance = 1;
                    break;
                }
                right->left->balance = 0;
                rotate_right(right, tree);
            }
            rotate_left(unbalanced, tree);
            break;
        }
    case -2: 
        {
            avl_node_t *left = unbalanced->left;

            if (left->balance == -1) {
                unbalanced->balance = 0;
                left->balance = 0;
            } else {
                switch (left->right->balance) {
                case 1:
                    unbalanced->balance = 0;
                    left->balance = -1;
                    break;
                case 0:
                    unbalanced->balance = 0;
                    left->balance = 0;
                    break;
                case -1:
                    unbalanced->balance = 1;
                    left->balance = 0;
                    break;
                }
                left->right->balance = 0;
                rotate_left(left, tree);
            }
            rotate_right(unbalanced, tree);
            break;
        }
    }
    insertion_succeeded(tree);
    return 0;
}

static int 
thread_unsafe_avl_tree_remove (avl_tree_t *tree,
        void *data_to_be_removed,
        void **actual_data_removed)
{
    avl_node_t *node, *to_be_deleted;
    avl_node_t *parent, *unbalanced;
    avl_node_t *left;
    avl_node_t *right;
    avl_node_t *next;
    int is_left;

    safe_pointer_set(actual_data_removed, NULL);

    /* being traversed, cannot access */
    if (tree->should_not_be_modified) {
        deletion_failed(tree);
        return EBUSY;
    }

    /* find the matching node first */
    node = avl_lookup_engine(tree, data_to_be_removed,
            &parent, &unbalanced, &is_left);

    /* not there */
    if (!node) {
        deletion_failed(tree);
        return ENODATA;
    }

    /* if we are here, we found it */
    safe_pointer_set(actual_data_removed, node->user_data);

    /* cache it for later freeing */
    to_be_deleted = node;

    parent = node->parent;
    left = node->left;
    right = node->right;

    if (!left)
        next = right;
    else if (!right)
        next = left;
    else
        next = get_first(right);

    if (parent) {
        is_left = (parent->left == node);
        set_child(next, parent, is_left);
    } else
        tree->root_node = next;

    if (left && right) {
        next->balance = node->balance;
        next->left = left;
        left->parent = next;
        if (next != right) {
            parent = next->parent;
            next->parent = node->parent;
            node = next->right;
            parent->left = node;
            next->right = right;
            right->parent = next;
            is_left = 1;
        } else {
            next->parent = parent;
            parent = next;
            node = parent->right;
            is_left = 0;
        }
        assert(parent != NULL);
    } else
        node = next;

    if (node)
        node->parent = parent;

    while (parent) {

        int balance;
            
        node = parent;
        parent = parent->parent;
        if (is_left) {
            is_left = (parent && (parent->left == node));
            balance = ++node->balance;
            if (balance == 0)           /* case 1 */
                continue;
            if (balance == 1) {         /* case 2 */
                goto END_OF_DELETE;
            }
            right = node->right;
            switch (right->balance) {
            case 0:                             /* case 3.1 */
                node->balance = 1;
                right->balance = -1;
                rotate_left(node, tree);
                goto END_OF_DELETE;
            case 1:                             /* case 3.2 */
                node->balance = 0;
                right->balance = 0;
                break;
            case -1:                    /* case 3.3 */
                switch (right->left->balance) {
                case 1:
                    node->balance = -1;
                    right->balance = 0;
                    break;
                case 0:
                    node->balance = 0;
                    right->balance = 0;
                    break;
                case -1:
                    node->balance = 0;
                    right->balance = 1;
                    break;
                }
                right->left->balance = 0;
                rotate_right(right, tree);
            }
            rotate_left(node, tree);
        } else {
            is_left = (parent && (parent->left == node));
            balance = --node->balance;
            if (balance == 0)
                continue;
            if (balance == -1) {
                goto END_OF_DELETE;
            }
            left = node->left;
            switch (left->balance) {
            case 0:
                node->balance = -1;
                left->balance = 1;
                rotate_right(node, tree);
                goto END_OF_DELETE;
            case -1:
                node->balance = 0;
                left->balance = 0;
                break;
            case 1:
                switch (left->right->balance) {
                case 1:
                    node->balance = 0;
                    left->balance = -1;
                    break;
                case 0:
                    node->balance = 0;
                    left->balance = 0;
                    break;
                case -1:
                    node->balance = 1;
                    left->balance = 0;
                    break;
                }
                left->right->balance = 0;
                rotate_left(left, tree);
            }
            rotate_right(node, tree);
        }
    }

END_OF_DELETE:
                
    free_avl_node(tree, to_be_deleted);
    if (tree->n <= 0) {
        assert(tree->root_node == NULL);
    } else {
        assert(tree->root_node != NULL);
    }
    deletion_succeeded(tree);
    return 0;
}

/*
 * morris traverse: preorder tree traversal without 
 * using recursion and without stack.  No extra storage
 * is needed, therefore it is frugal in memory usage.
 *
 * For as long as the user function keeps on returning 
 * zero, we keep on calling it for every node (user data)
 * in the tree.
 *
 * One very important point here.  Since morris traversal
 * changes pointers, we cannot stop until we traverse the
 * entire tree so that all pointers are recovered back.
 * Therefore, if the user traverse function returns a non 0,
 * we can NOT stop traversing; we must continue.  However,
 * once the error occurs, we simply stop calling the user
 * function for the rest of the traversal.
 */
static int
thread_unsafe_avl_tree_morris_traverse (avl_tree_t *tree, avl_node_t *root,
        traverse_function_pointer tfn,
        void *p0, void *p1, void *p2, void *p3)
{
    int failed = 0;
    avl_node_t *current;

    /*
     * in case the traversal funtion attempts to 
     * change the tree from under us
     */
    tree->should_not_be_modified = true;

    /* if root is not given, the entire tree will be traversed */
    if (NULL == root) root = tree->root_node;

    while (root) {
        if (root->left == NULL) {
            if (0 == failed) {
                failed = tfn(tree, root, root->user_data, p0, p1, p2, p3);
            }
            root = root->right;
        } else {
            current = root->left;
            while (current->right && (current->right != root)) {
                current = current->right;
            }
            if (current->right == root) {
                current->right = NULL;
                root = root->right;
            } else {
                if (0 == failed) {
                    failed = tfn(tree, root, root->user_data, p0, p1, p2, p3);
                }
                current->right = root;
                root = root->left;
            }
        }
    }
    tree->should_not_be_modified = false;
    return failed;
}

/*
 * Very similar to morris traversal.
 */
static int
thread_unsafe_avl_tree_iterate (avl_tree_t *tree, avl_node_t *root,
        traverse_function_pointer tfn,
        void *p0, void *p1, void *p2, void *p3)
{
    avl_node_t *node;
    int failed = 0;

    if (NULL == root) root = tree->root_node;
    node = root;
    tree->should_not_be_modified = true;
    while (true) {
        if (node->left_visited) {
            if (node->right_visited) {
                node->left_visited = node->right_visited = false;
                if (node == root) break;
                node = node->parent;
            } else {
                node->right_visited = true;
                if (node->right) {
                    node = node->right;
                }
            }
        } else {
            if (node->right_visited) {
                ERROR(&avl_debug, "left NOT done but right done\n");
            } else {
                if (!failed /* && (node != root) */) {
                    failed = tfn(tree, node, node->user_data, p0, p1, p2, p3);
                }
                node->left_visited = true;
                if (node->left) {
                    node = node->left;
                }
            }
        }
    }
    tree->should_not_be_modified = false;
    return failed;
}

/*
 * destroy everything below & including the root node.
 * If 'root' is NULL, the entire tree is destroyed.
 */
static int
thread_unsafe_iterative_destroy (avl_tree_t *tree, avl_node_t *root,
        destruction_handler_t dh_fptr, void *extra_arg)
{
    avl_node_t *n, *parent;
    int delete_count;

    /*
     * in case the destruction handler tries
     * to change the tree from under us.
     */
    tree->should_not_be_modified = true;

    if (NULL == root) root = tree->root_node;
    parent = (root == tree->root_node) ? NULL : root->parent;
    delete_count = 0;
    while (root) {
        if (root->left) {
            n = root->left;
            root->left = NULL;
        } else if (root->right != NULL) {
            n = root->right;
            root->right = NULL;
        } else {
            n = root->parent;
            if (dh_fptr) dh_fptr(root->user_data, extra_arg);
            free_avl_node(tree, root);
            delete_count++;
        }
        root = n;
    }
    if (parent) {
        parent->left = parent->right = NULL;
    } else {
        tree->root_node = NULL;
    }
    tree->should_not_be_modified = false;
    return delete_count;
}

/**************************** Initialize *************************************/

PUBLIC int
avl_tree_init (avl_tree_t *tree,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        object_comparer cmpf,
        mem_monitor_t *parent_mem_monitor)
{
    if (NULL == cmpf) return EINVAL;
    MEM_MONITOR_SETUP(tree);
    LOCK_SETUP(tree);
    STATISTICS_SETUP(tree);

    reset_stats(tree);
    tree->cmpf = cmpf;
    tree->n = 0;
    tree->root_node = NULL;
    tree->should_not_be_modified = false;

    return 0;
}

PUBLIC void
avl_tree_debug_set_level (int level)
{
    debug_module_block_set_level(&avl_debug, level);
}

PUBLIC void
avl_tree_debug_set_module_name (char *name)
{
    debug_module_block_set_module_name(&avl_debug, name);
}

PUBLIC void
avl_tree_debug_set_reporting_function (debug_reporting_function drf)
{
    debug_module_block_set_reporting_function(&avl_debug, drf);
}

/**************************** Insert *****************************************/

PUBLIC int
avl_tree_insert (avl_tree_t *tree,
        void *data,
        void **present_data,
        boolean overwrite_if_present)
{
    int failed;

    OBJ_WRITE_LOCK(tree);
    failed = thread_unsafe_avl_tree_insert(tree,
                data, present_data, overwrite_if_present);
    OBJ_WRITE_UNLOCK(tree);
    return failed;
}

/**************************** Search *****************************************/

PUBLIC int 
avl_tree_search (avl_tree_t *tree, 
        void *data_to_be_searched,
        void **present_data)
{
    int failed;
    avl_node_t *parent, *unbalanced, *node;
    int is_left;

    OBJ_READ_LOCK(tree);
    node = avl_lookup_engine(tree, data_to_be_searched, 
                &parent, &unbalanced, &is_left);
    if (node) {
        safe_pointer_set(present_data, node->user_data);
        search_succeeded(tree);
        failed = 0;
    } else {
        safe_pointer_set(present_data, NULL);
        search_failed(tree);
        failed = ENODATA;
    }
    OBJ_READ_UNLOCK(tree);
    return failed;
}

/**************************** Remove *****************************************/

PUBLIC int
avl_tree_remove (avl_tree_t *tree,
        void *data_to_be_removed,
        void **actual_data_removed)
{
    int failed;

    OBJ_WRITE_LOCK(tree);
    failed = thread_unsafe_avl_tree_remove(tree,
                data_to_be_removed, actual_data_removed);
    OBJ_WRITE_UNLOCK(tree);
    return failed;
}

/**************************** Traverse ***************************************/

PUBLIC int
avl_tree_morris_traverse (avl_tree_t *tree, avl_node_t *root,
        traverse_function_pointer tfn,
        void *p0, void *p1, void *p2, void *p3)
{
    int failed;

    OBJ_READ_LOCK(tree);
    failed = thread_unsafe_avl_tree_morris_traverse(tree, root,
                tfn, p0, p1, p2, p3);
    OBJ_READ_UNLOCK(tree);
    return failed;
}

PUBLIC int
avl_tree_iterate (avl_tree_t *tree, avl_node_t *root,
        traverse_function_pointer tfn,
        void *p0, void *p1, void *p2, void *p3)
{
    int failed;

    OBJ_READ_LOCK(tree);
    failed = thread_unsafe_avl_tree_iterate(tree, root,
                tfn, p0, p1, p2, p3);
    OBJ_READ_UNLOCK(tree);
    return failed;
}

/**************************** Destroy ****************************************/

PUBLIC void
avl_tree_destroy (avl_tree_t *tree,
        destruction_handler_t dh_fptr, void *extra_arg)
{
    int old_count, deleted;

    OBJ_WRITE_LOCK(tree);
    old_count = tree->n;
    deleted =
        thread_unsafe_iterative_destroy(tree, tree->root_node,
            dh_fptr, extra_arg);
    assert(tree->n == 0);
    assert(old_count == deleted);
    tree->root_node = NULL;
    tree->cmpf = NULL;
    OBJ_WRITE_UNLOCK(tree);
    LOCK_OBJ_DESTROY(tree);
    memset(tree, 0, sizeof(avl_tree_t));
}

#ifdef __cplusplus
} // extern C
#endif 



