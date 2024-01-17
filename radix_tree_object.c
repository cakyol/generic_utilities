
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

#include "radix_tree_object.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PUBLIC

#define LO_NIBBLE(value)            ((value) & 0xF)
#define HI_NIBBLE(value)            ((value) >> 4)

static inline void 
radix_tree_node_init (radix_tree_node_t *node, int value)
{
    memset(node, 0, sizeof(radix_tree_node_t));
    node->value = value;
}

static inline radix_tree_node_t *
radix_tree_new_node (radix_tree_t *rtp, int value)
{
    radix_tree_node_t *node;

    node = (radix_tree_node_t*)
	MEM_MONITOR_ZALLOC(rtp, sizeof(radix_tree_node_t));
    if (node) {
        node->value = value;
    }
    return node;
}

static inline radix_tree_node_t *
radix_tree_add_nibble (radix_tree_t *rtp, radix_tree_node_t *parent, int nibble)
{
    radix_tree_node_t *node;

    node = parent->children[nibble];

    /* an entry already exists */
    if (node) {
        return node;
    }

    /* new entry */
    node = radix_tree_new_node(rtp, nibble);
    if (node) {
        node->parent = parent;
        parent->children[nibble] = node;
        parent->n_children++;
        rtp->node_count++;
    }

    return node;
}

static radix_tree_node_t *
radix_tree_add_byte (radix_tree_t *rtp, radix_tree_node_t *parent, int value)
{
    radix_tree_node_t *new_node;

    new_node = radix_tree_add_nibble(rtp, parent, LO_NIBBLE(value));
    if (NULL == new_node) {
        return NULL;
    }
    new_node = radix_tree_add_nibble(rtp, new_node, HI_NIBBLE(value));
    return new_node;
}

static radix_tree_node_t *
radix_tree_node_insert (radix_tree_t *rtp, byte *key, int key_length)
{
    radix_tree_node_t *new_node = NULL, *parent;

    parent = &rtp->radix_tree_root;
    while (key_length-- > 0) {
        new_node = radix_tree_add_byte(rtp, parent, *key);
        if (new_node) {
            parent = new_node;
            key++;
        } else {
            return NULL;
        }
    }
    return new_node;
}

static radix_tree_node_t *
radix_tree_node_find (radix_tree_t *rtp, byte *key, int key_length)
{
    radix_tree_node_t *node = &rtp->radix_tree_root;

    while (1) {

        /* follow hi nibble */
        node = node->children[LO_NIBBLE(*key)];

        /*
         * This wont happen since depth is always an even number
         * (2 nibbles per byte).
         */
        // if (NULL == node) return NULL;

        /* follow lo nibble */
        node = node->children[HI_NIBBLE(*key)];
        if (NULL == node) return NULL;

        /* have we seen the entire pattern */
        if (--key_length <= 0) return node;

        /* next byte */
        key++;
    }

    /* not found */
    return NULL;
}

/*
** This is tricky, be careful
*/
static void 
radix_tree_remove_node (radix_tree_t *rtp, radix_tree_node_t *node)
{
    radix_tree_node_t *parent;

    /* do NOT delete root node, that is the ONLY one with no parent */
    while (node->parent) {

        /* if I am DIRECTLY in use, I cannot be deleted */
        if (node->user_data) return;

        /* if I am IN-DIRECTLY in use, I still cannot be deleted */
        if (node->n_children > 0) return;

        /* clear the parent pointer which points to me */
        parent = node->parent;
        parent->children[node->value] = NULL;
        parent->n_children--;

        /* delete myself */
        MEM_MONITOR_FREE(node);
        rtp->node_count--;

        /* go up one more parent & try again */
        node = parent;
    }
}

static int
thread_unsafe_radix_tree_insert (radix_tree_t *rtp,
        void *key, int key_length, 
        void *data_to_be_inserted, void **present_data)
{
    radix_tree_node_t *node;

    /* assume failure */
    safe_pointer_set(present_data, NULL);

    /* should not store NULL data */
    if (NULL == data_to_be_inserted) return EINVAL;

    /* being traversed, cannot access */
    if (rtp->should_not_be_modified) return EBUSY;

    node = radix_tree_node_insert(rtp, key, key_length);
    if (node) {

        /*
         * if the node returned here is a NEW one, it will NOT have
         * its user_data assigned (it will be NULL).  If it is NOT
         * null, then it indicates that the node was already in the
         * tree, ie an already existing entry.
         */
        if (node->user_data) {
            safe_pointer_set(present_data, node->user_data);
        } else {
            node->user_data = data_to_be_inserted;
        }
        return 0;
    }

    return ENOMEM;
}

static int
thread_unsafe_radix_tree_search (radix_tree_t *rtp,
        void *key, int key_length,
        void **present_data)
{
    radix_tree_node_t *node;
    
    /* assume failure */
    safe_pointer_set(present_data, NULL);

    node = radix_tree_node_find(rtp, key, key_length);
    if (node && node->user_data) {
        safe_pointer_set(present_data, node->user_data);
        return 0;
    }

    return ENODATA;
}

static int 
thread_unsafe_radix_tree_remove (radix_tree_t *rtp,
        void *key, int key_length, 
        void **removed_data)
{
    radix_tree_node_t *node;

    /* assume failure */
    safe_pointer_set(removed_data, NULL);

    /* being traversed, cannot access */
    if (rtp->should_not_be_modified) return EBUSY;

    node = radix_tree_node_find(rtp, key, key_length);
    if (node && node->user_data) {
        safe_pointer_set(removed_data, node->user_data);
        node->user_data = NULL;
        radix_tree_remove_node(rtp, node);
        return 0;
    }
    return ENODATA;
}

/**************************** Public *****************************************/

PUBLIC int 
radix_tree_init (radix_tree_t *rtp,
        boolean make_it_thread_safe,
        boolean enable_statistics,
        mem_monitor_t *parent_mem_monitor)
{
    MEM_MONITOR_SETUP(rtp);
    LOCK_SETUP(rtp);
    STATISTICS_SETUP(rtp);

    rtp->node_count = 0;
    radix_tree_node_init(&rtp->radix_tree_root, 0);

    return 0;
}

PUBLIC int
radix_tree_insert (radix_tree_t *rtp,
        void *key, int key_length,
        void *data_to_be_inserted, void **present_data)
{
    int failed;

    OBJ_WRITE_LOCK(rtp);
    failed = thread_unsafe_radix_tree_insert(rtp, key, key_length,
                data_to_be_inserted, present_data);
    OBJ_WRITE_UNLOCK(rtp);
    return failed;
}

PUBLIC int
radix_tree_search (radix_tree_t *rtp,
        void *key, int key_length,
        void **present_data)
{
    int failed;

    OBJ_READ_LOCK(rtp);
    failed = thread_unsafe_radix_tree_search(rtp, key, key_length, present_data);
    OBJ_READ_UNLOCK(rtp);
    return failed;
}

PUBLIC int
radix_tree_remove (radix_tree_t *rtp,
        void *key, int key_length,
        void **removed_data)
{
    int failed;

    OBJ_WRITE_LOCK(rtp);
    failed = thread_unsafe_radix_tree_remove(rtp, key, key_length, removed_data);
    OBJ_WRITE_UNLOCK(rtp);
    return failed;
}

/*
 * This traverse function does not use recursion or a
 * separate stack.  This is very useful for very deep
 * trees where we may run out of stack or memory.  This
 * method will never run out of memory but will just take
 * longer.
 *
 * One very important point here.  Since this traversal
 * changes node values, we cannot stop until we traverse the
 * entire tree so that all nodes are reverted back to normal.
 * Therefore, if the user traverse function returns a non 0,
 * we can NOT stop traversing; we must continue.  However,
 * once the error occurs, we simply stop calling the user
 * function for the rest of the tree.
 */
PUBLIC void
radix_tree_traverse (radix_tree_t *rtp, traverse_function_pointer tfn,
        void *extra_arg_1, void *extra_arg_2)
{
    radix_tree_node_t *node, *prev;
    byte *key;
    void *key_len;
    int index = 0;
    int failed = 0;

    /*
     * already being traversed.  Normally, since a traversal is a 'read'
     * operation, this should be allowed.  But the traversal in this function
     * changes nodes (and restores) them.  So a recursive traveral cannot 
     * be allowed.
     */
    if (rtp->should_not_be_modified) return;

    /* start traversal */
    rtp->should_not_be_modified = 1;

    key = malloc(8192);
    if (NULL == key) return;
    OBJ_READ_LOCK(rtp);
    node = &rtp->radix_tree_root;
    node->current = 0;
    while (node) {
        if (node->current < NTRIE_ALPHABET_SIZE) {
            if (index >= 1) {
                if (index & 1) {
                    /* lo nibble */
                    key[(index-1)/2] = node->value;
                } else {
                    /* hi nibble */
                    key[(index-1)/2] |= (node->value << 4);
                }
            }
            prev = node;
            if (node->children[node->current]) {
                node = node->children[node->current];
                index++;
            }
            prev->current++;
        } else {
            // Do your thing with the node here as long as no error occured so far
            key_len = integer2pointer(index/2);
            if (node->user_data && (0 == failed)) {
                failed = tfn(rtp, node, node->user_data, key, key_len,
                    extra_arg_1, extra_arg_2);
            }
            node->current = 0;  // Reset counter for next traversal.
            node = node->parent;
            index--;
        }
    }
    OBJ_READ_UNLOCK(rtp);

    /* ok traversal finished */
    rtp->should_not_be_modified = 0;

    free(key);
}

PUBLIC void
radix_tree_destroy (radix_tree_t *rtp)
{
    SUPPRESS_UNUSED_VARIABLE_COMPILER_WARNING(rtp);
}

#ifdef __cplusplus
} // extern C
#endif 


