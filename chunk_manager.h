
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

#ifndef __CHUNK_MANAGER_H__
#define __CHUNK_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"

/******************************************************************************
 *
 * This is a chunk manager which manages a many same sized chunks, 
 * dynamically allocated & freed.  It is like malloc & free but much
 * faster since most chunks are pre-allocated and disbursing them
 * is therefore quite fast.  Comparing the speed to malloc/free,
 * for chunk sizes of up to about 64 bytes, the performance is up
 * to about 2.5 times faster.  For chunk sizes above 64 bytes, it is
 * up to about 7-8 times faster.
 *
 * There is one VERY important requirement of this implementation.
 * It is the fact that when this dynamically grows, the addresses of
 * all previously allocated chunks MUST stay the same.  Therefore,
 * a simple realloc scheme could not be used since at some point,
 * a realloc may change the adresses completely, violating this
 * principle.  The reason this is required is that if the user
 * caches the pointer values of these chunks, they must never change.
 *
 * To be able to provide the above, the concept of a 'chunk group' is
 * defined where all the addresses in that group always stay the same.
 * In each group, a big block of malloced area is divided into many chunks
 * and are maintained.  If chunks are all used up and more is needed,
 * we cannot simply realloc the block and expand it.  This is because
 * the realloc may change the addresses and this would not work.  To
 * provide this expansion requirement, an expanding list of chunk groups
 * is therefore maintained.  When all the chunks in a group are exhausted,
 * a new group is created.  When a new chunk group is created, all the
 * available chunk addresses are added to the list of free chunks in the
 * main chunk manager structure.  This causes the chunk manager to
 * pause and add new chunks to the free chunks stack when the current
 * set of chunks are depleted.
 *
 * A bigger challenge is when 'trimming' the structure is needed.
 * This is when the user decides that there are a lot of free chunks
 * around which are not needed and may have to be returned back to
 * memory.  When this is requested, all the groups are scanned and
 * for the ones who have all their chunks unused, each chunk is
 * removed off the free chunks stack and the entire group deleted.
 * This can be a bit tricky and time consuming and is therefore not
 * automatically performed but left to the user as to when it needs
 * to be run.
 *
 */

typedef struct chunk_header_s chunk_header_t;
typedef struct chunk_group_s chunk_group_t;
typedef struct chunk_manager_s chunk_manager_t;

struct chunk_manager_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;

    /*
     * size of each chunk the user wants and how
     * many bytes it internally occupies
     */
    int chunk_size;
    int actual_chunk_size;

    /* how many chunks per group is needed */
    int chunks_per_group;

    /*
     * a linked list of all the free chunks in all the
     * groups and their count
     */
    chunk_header_t *free_chunks_list;
    int n_cmgr_free;

    /* a linked list of all the groups */
    chunk_group_t *groups;

};

/*
 * Redefine these as per your own requirements
 */

#define MIN_CHUNK_SIZE          8
#define MAX_CHUNK_SIZE          256
#define MIN_CHUNKS_PER_GROUP    64
#define MAX_CHUNKS_PER_GROUP    (0xFFFF)

/*
 * initialize the chunk manager.  'chunk_size' is the size
 * of each chunk guaranteed when returned to the user.
 * 'chunks_per_group' defines how many chunks will be created
 * all at once, in advance of calling the allocation function.
 * The higher this number, the more efficient the chunk manager
 * will run, but if you do not use all the chunks, memory will
 * have been wastefully allocated.  It is up to the user to
 * fine tune this value.
 *
 * Return value is 0 for success or a non zero errno if the
 * function fails.  The only failures which can occur during
 * initialization are if either the chunk_size specified or
 * the chunks_per_group specified is outside the valid limits.
 * The memory block allocations actually do NOT start until
 * the very first call to 'chunk_manager_alloc" is made.
 */
extern int
chunk_manager_init (chunk_manager_t *cmgrp,
    boolean make_it_thread_safe,
    int chunk_size, int chunks_per_group,
    mem_monitor_t *parent_mem_monitor);

/*
 * returns a pointer to a memory block with a size specified
 * at the initialization of the chunk manager.  Do NOT access
 * past either end of this chunk.  NULL will be returned if
 * no more chunks are available or cannot be created.
 */
extern void *
chunk_alloc (chunk_manager_t *cmgrp);

/*
 * Frees a chunk which was supplied by the call to 'chunk_manager_alloc'.
 * Do NOT pass any other pointer to this function except only the ones
 * returned by the call to chunk_manager_alloc, with the same manager.
 */
extern void
chunk_free (void *chunk);

/*
 * When chunks are allocated, they are also internally cached so that
 * they can be re allocated very quickly.  So, if the user allocates
 * lots of chunks and then frees them up, even tho they are free to
 * be used, they will linger around in the cache and will not have
 * been returned to the OS.
 *
 * To eliminate this, if the user decides that the caches can be
 * cleared and the chunks can be returned back to the OS so that
 * the chunk manager does not become a memory hog (becoz of its cache),
 * this function can be called.
 *
 * Return value is the number of 'chunk_group_t' groups returned
 * back to the OS.
 */
extern int
chunk_manager_trim (chunk_manager_t *cmgrp);

/*
 * destroys the chunk manager object.  The object can no longer
 * be used until the next initialization.  The entire memory it uses
 * will be returned back to the OS.
 */
extern void
chunk_manager_destroy (chunk_manager_t *cmgrp);

#ifdef __cplusplus
} // extern C
#endif

#endif // __CHUNK_MANAGER_H__


