
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
*
* An fast buffer allocator.  The only limitation it 
* has is that the max allocatable size is limited to 64k bytes
* exactly (0xFFFF) bytes.
*
* The allocator is initialized with a list of (size, count) tuples
* which is user specified.  For example, if the user initializes
* the object with the following set of tuples (first number is the
* size of buffer, the second is how many of those buffers are required).
* Any negative number combination TOGETHER will be the terminating entry.
*
* 128, 1000
* 512, 2000
* 1500, 1000
* 9000, 500
* 150000, 10
* -2, -4 <<< any 2 negative numbers terminate the array.
*
* It means that the user requires 1000 buffers of 128 bytes each and
* 2000 buffers of 512 bytes each and 1000 buffers of 1500 bytes each and
* 500 buffers of 9000 bytes each.  The LAST entry of 10 buffers of 
* 150,000 bytes each can NOT be honored since the maximum size specified
* can not ever exceed 64k (0xFFFF, 65,535).
*
* Note that buffers are pre-carved and therefore total needed memory
* can quickly add up to a lot of bytes.  Be careful during initialization
* not to exceed the total memory size of your system.
*
* The object will do its best to allocate as much as it can but will
* stop if memory is exhausted.  It will not crash but unallocatable
* pools will simply be set to NULL.
*
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __BUFFER_MANAGER_H__
#define __BUFFER_MANAGER_H__

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"
#include "debug_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * During initialization, signifies a buffer pool.  'size'
 * is the number of bytes each block in this pool is and
 * 'count' signifies how many of such blocks are in this pool.
 */
typedef struct size_count_tuple_s {

    int size;
    int count;

} size_count_tuple_t;

typedef struct buffer_s buffer_t;
typedef struct buffer_pool_s buffer_pool_t;
typedef struct buffer_manager_s buffer_manager_t;

/*
 * A buffer allocated from a pool.
 * The user sees ONLY the part past the 'start'.
 * This will be 8 byte aligned.
 *
 * The 'next' pointer simply points to the next free available
 * buffer.  The 'poolp' points to the pool this buffer belongs to,
 * where it should be returned to when freed.  This makes it
 * a very fast free operation to simply re-insert it to the head
 * of the free list of that pool.
 *
 * Note that user does NOT see or need anything before 'data'.
 */
struct buffer_s {

    /* the memory pool this buffer belongs to (used when freeing) */
    buffer_pool_t *poolp;

    /* next free buffer in the list (used for allocating) */
    buffer_t *next;

    /*
     * This is what the user sees, ONLY,
     * an 8 byte aligned buffer of memory
     */
    unsigned char data [0] __attribute__((aligned(8)));
};

/*
 * Defines ONE memory pool
 */
struct buffer_pool_s {

    /* what buffer manager this pool belongs to */
    buffer_manager_t *bmp;

    /*
     * The original buffer size the user requested.
     * Note that as the pool is populated, this will be adjusted
     * to the next multiple of 8 bytes, and stored in 'actual_buffer_size'.
     */
    int specified_size;

    /*
     * Adjusted size of each buffer after the user requested size
     * has been increased to the next multiple of 8 bytes AND the
     * pre buffer header has been added to it.
     */
    int actual_buffer_size;

    /* how many buffers this particular pool has */
    int buffer_count;

    /*
     * The entire malloced block for this pool.
     * When destroying a pool, the entire set of
     * buffers can be destroyed just by freeing this single
     * block.  One does not have to free traversing
     * any lists, one buffer at a time.
     */
    void *block;

    /* linked list of all the buffers */
    buffer_t *head;
};

/*
 * Adjust this to taste, but dont be
 * ridiculous or your mallocs WILL fail.
 */
#define MAX_POOLS           16

/*
 * The main user structure
 */
struct buffer_manager_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;

    /* how many pools are in this buffer manager */
    int num_pools;

    /* the buffer size of the pool with the largest buffer size */
    int max_size;

    /*
     * creates a size -> pool number number lookup
     * table for incredibly fast buffer allocation.
     */
    byte *size_lookup_table;

    /* The array of pools for this memory allocator */
    buffer_pool_t pools [MAX_POOLS];
};

/*
 * Initialize the buffer pools
 */
extern int
buffer_manager_initialize (buffer_manager_t *bmp,
        bool make_it_thread_safe,
        size_count_tuple_t tuples [],
        mem_monitor_t *parent_mem_monitor);

extern void *
buffer_allocate (buffer_manager_t *bmp, int size);

extern void
buffer_free (void *ptr);

extern void
buffer_manager_destroy (buffer_manager_t *bmp);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* __BUFFER_MANAGER_H__ */


