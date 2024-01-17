
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

/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
**
** @@@@@ NIBBLE RADIX TREE/TRIE
**
** This trie can be used for ANY object of ANY size.
**
** It represents ALL objects simply as a sequence of
** nibbles (half bytes).  This keeps the alphabet size
** always to 16 (4 bits) and thus saving a lot of storage.
**
** Since it uses a node per nibble, operations on a single byte
** always takes 2 actions; One for the LSB nibble and one for 
** the MSB nibble.  This way, anything can be parsed one byte
** at a time.  It doubles the lookup time but saves a lot
** in storage.  But since each lookup is now array based, it
** is extremely fast.  The disadvantage of using 2 traversal
** per byte is completely offseted by the speed of an array
** lookup per each node.
**
** Every lookup takes EXACTLY O(2k) array accesses, where 'k'
** is the length of the key, NOT the number of nodes.  This
** structure is probably the fastest way to insert, find and
** delete data.  Also remember that these are not 'comparisons'
** but very fast array accesses.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __RADIX_TREE_OBJECT_H__
#define __RADIX_TREE_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"

#define NTRIE_LOW_VALUE         0
#define NTRIE_HI_VALUE          (0xF)
#define NTRIE_ALPHABET_SIZE     (NTRIE_HI_VALUE - NTRIE_LOW_VALUE + 1)

typedef struct radix_tree_node_s radix_tree_node_t;

struct radix_tree_node_s {

    radix_tree_node_t* parent;
    radix_tree_node_t *children [NTRIE_ALPHABET_SIZE];
    void *user_data;
    byte current;       // used for non recursive traversal
    byte value;
    byte n_children;
};

typedef struct radix_tree_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    STATISTICS_VARIABLES;

    int should_not_be_modified;
    int node_count;
    radix_tree_node_t radix_tree_root;

} radix_tree_t;

extern int 
radix_tree_init (radix_tree_t *ntp, 
        boolean make_it_thread_safe,
        boolean enable_statistics,
        mem_monitor_t *parent_mem_monitor);

extern int 
radix_tree_insert (radix_tree_t *ntp,
        void *key, int key_length, 
        void *data_to_be_inserted,
        void **present_data);

extern int 
radix_tree_search (radix_tree_t *ntp, 
        void *key, int key_length, 
        void **present_data);

extern int 
radix_tree_remove (radix_tree_t *ntp, 
        void *key, int key_length, 
        void **data_removed);

extern void
radix_tree_traverse (radix_tree_t *ntp, traverse_function_pointer tfn,
        void *extra_arg_1, void *extra_arg_2);

extern void 
radix_tree_destroy (radix_tree_t *ntp);

#ifdef __cplusplus
} // extern C
#endif 

#endif // __RADIX_TREE_OBJECT_H__


