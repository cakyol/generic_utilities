
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

#include "chunk_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Variable sized chunk header.
 * Each chunk is added to a linked list (of free chunks) on the
 * main structure.
 */
struct chunk_header_s {

    /*
     * The chunk group I belong to so that when I am freed up,
     * I will return to the same group.
     */
    chunk_group_t *my_group;

    /* next free chunk */
    chunk_header_t *next_chunk_header;

    /*
     * this is what is returned back to the user and it must
     * always be 8 bytes aligned, hence the use of long long int.
     */
    long long int data [0];
};

/*
 * Each group has a big memory block which is divided into chunk size
 * sections.  Each chunk is then added to a linked list (of free chunks)
 * on the main structure.
 */
struct chunk_group_s {

    /* The chunk manager I belong to */
    chunk_manager_t *my_manager;

    /* big bulk of the memory, all chunks adjacent in one big block */
    void *chunks_block;

    /* how many free chunks are in this group */
    int n_grp_free;

    /* next group in list */
    chunk_group_t *next_chunk_group;

};

static int
chunk_manager_add_group_failed (chunk_manager_t *cmgrp)
{
    byte *bp;
    chunk_header_t *chp;
    chunk_group_t *cgp;
    int i;

    /* allocate chunk group structure itself */
    cgp = MEM_MONITOR_ALLOC(cmgrp, sizeof(chunk_group_t));
    if (NULL == cgp) return ENOMEM;

    /* allocate the big chunk block */
    cgp->chunks_block = MEM_MONITOR_ALLOC(cmgrp,
            (cmgrp->actual_chunk_size * cmgrp->chunks_per_group));
    if (NULL == cgp->chunks_block) {
        MEM_MONITOR_FREE(cgp);
        return ENOMEM;
    }

    /*
     * run thru the newly allocated block and partition each chunk
     * and add it to the head of the free chunks list in the main
     * manager.
     */
    bp = cgp->chunks_block;
    for (i = 0; i < cmgrp->chunks_per_group; i++) {
        chp = (chunk_header_t*) bp;
        chp->my_group = cgp;
        chp->next_chunk_header = cmgrp->free_chunks_list;
        cmgrp->free_chunks_list = chp;
        bp += cmgrp->actual_chunk_size;
    }

    /* update group related stuff */
    cgp->my_manager = cmgrp;
    cgp->n_grp_free = cmgrp->chunks_per_group;

    /* update the main manager related stuff */
    // cmgrp->n_cmgr_free += cmgrp->chunks_per_group;

    /*
     * Everything in the group is now initialised, now
     * add the new group to the head of groups list
     */
    cgp->next_chunk_group = cmgrp->groups;
    cmgrp->groups = cgp;

    return 0;
}

/*
 * Grab a chunk from the head of the free chunks list,
 * and return it to the caller, adjusting counters & head.
 */
static inline void *
thread_unsafe_chunk_manager_alloc (chunk_manager_t *cmgrp)
{
    chunk_header_t *chp;

    /* pop the first chunk from the free chunks list */
    //if (cmgrp->n_cmgr_free > 0) {
    chp = cmgrp->free_chunks_list;
    if (chp) {
        cmgrp->free_chunks_list = chp->next_chunk_header;
        (chp->my_group->n_grp_free)--;
        //(cmgrp->n_cmgr_free)--;
        return &(chp->data[0]);
    }

    /*
     * if we are here, no more free chunks left, so create a new
     * group and recursively call the function again.
     */
    if (chunk_manager_add_group_failed(cmgrp)) {
        return null;
    }

    /* new group created, recursive call will return successfully */
    return
        thread_unsafe_chunk_manager_alloc(cmgrp);
}

/*
 * This function tries to return unused chunks back to the OS.
 * How does it do this ?  It makes a single pass thru all the
 * free chunks checking their groups.  If the group that
 * the chunk belongs to has all its chunks free, this indicates
 * that that specific group will be deleted and hence this chunk
 * also must be removed from the main manager's list.  During this
 * pass, the functions reconstructs the new free list of chunks by
 * excluding the ones which will be deleted (since their group
 * will be deleted).
 *
 * Once this pass finishes, then we also make a single pass thru
 * the groups and free the ones up whose free chunk count is
 * complete.
 */
static int
thread_unsafe_chunk_manager_trim (chunk_manager_t *cmgrp)
{
    chunk_header_t *chp, *next_chp;
    chunk_group_t *cgp, *next_cgp;
    int chunks_tobe_freed, grps_tobe_freed;

    /*
     * since we free up entire groups of chunks, there must at least
     * be cmgrp->chunks_per_group free chunks before we even bother
     * checking.  If this is not the case, it is impossible for any
     * group to have all its chunks free.
     */
    //if (cmgrp->n_cmgr_free < cmgrp->chunks_per_group)
        //return 0;

    chunks_tobe_freed = grps_tobe_freed = 0;
    chp = cmgrp->free_chunks_list;
    cmgrp->free_chunks_list = null;
    //cmgrp->n_cmgr_free = 0;
    while (chp) {

        next_chp = chp->next_chunk_header;

        /*
         * if this chunk does not belong to a group whose chunks
         * are all free (will be deleted), then re-add it to the
         * free list.  The rest will be left 'dangling' but they
         * will soon be all deleted anyway so it does not matter.
         */
        if (chp->my_group->n_grp_free < cmgrp->chunks_per_group) {
            chp->next_chunk_header = cmgrp->free_chunks_list;
            cmgrp->free_chunks_list = chp;
            //(cmgrp->n_cmgr_free)++;
        } else {
            chunks_tobe_freed++;
        }

        chp = next_chp;
    }

    /*
     * now free up all the groups whose chunks are all free
     * and which we just took off the main managers' free list
     */
    if (chunks_tobe_freed) {
        cgp = cmgrp->groups;
        cmgrp->groups = null;
        while (cgp) {
            next_cgp = cgp->next_chunk_group;
            if (cgp->n_grp_free < cmgrp->chunks_per_group) {
                cgp->next_chunk_group = cmgrp->groups;
                cmgrp->groups = cgp;
            } else {
                MEM_MONITOR_FREE(cgp->chunks_block);
                MEM_MONITOR_FREE(cgp);
                grps_tobe_freed++;
            }
            cgp = next_cgp;
        }
    }

    return grps_tobe_freed;
}

/***************************** 80 column separator ****************************/

PUBLIC int
chunk_manager_init (chunk_manager_t *cmgrp,
    boolean make_it_thread_safe,
    int chunk_size, int chunks_per_group,
    mem_monitor_t *parent_mem_monitor)
{
    /* basic sanity checks */
    if ((chunk_size < MIN_CHUNK_SIZE) ||
        (chunk_size > MAX_CHUNK_SIZE)) {
            return EINVAL;
    }
    if ((chunks_per_group < MIN_CHUNKS_PER_GROUP) ||
        (chunks_per_group > MAX_CHUNKS_PER_GROUP)) {
            return EINVAL;
    }

    /* clear absolutely everything */
    memset(cmgrp, 0, sizeof(chunk_manager_t));

    MEM_MONITOR_SETUP(cmgrp);
    LOCK_SETUP(cmgrp);

    /* align the size to the next 8 bytes */
    cmgrp->chunk_size = chunk_size;
    cmgrp->actual_chunk_size =
        ((chunk_size + 7) & ~7) + sizeof(chunk_header_t);

    cmgrp->chunks_per_group = chunks_per_group;

    return 0;
}

PUBLIC void *
chunk_alloc (chunk_manager_t *cmgrp)
{
    void *ptr;

    OBJ_WRITE_LOCK(cmgrp);
    ptr = thread_unsafe_chunk_manager_alloc(cmgrp);
    OBJ_WRITE_UNLOCK(cmgrp);

    return ptr;
}

PUBLIC void
chunk_free (void *chunk)
{
    chunk_header_t *chp;
    chunk_manager_t *cmgrp;

    /* get the hidden chunk header and the chunk manager pointer */
    chp = (chunk_header_t*) (((byte*) chunk) - sizeof(chunk_header_t));
    cmgrp = chp->my_group->my_manager;

    OBJ_WRITE_LOCK(cmgrp);

    /* group is being returned a chunk */
    (chp->my_group->n_grp_free)++;

    /* main chunk manager is being returned one */
    //(cmgrp->n_cmgr_free)++;

    /* place it back into the head of free chunks list */
    chp->next_chunk_header = cmgrp->free_chunks_list;
    cmgrp->free_chunks_list = chp;

    OBJ_WRITE_UNLOCK(cmgrp);
}

PUBLIC int
chunk_manager_trim (chunk_manager_t *cmgrp)
{
    int grps_tobe_freed;

    OBJ_WRITE_LOCK(cmgrp);
    grps_tobe_freed = thread_unsafe_chunk_manager_trim(cmgrp);
    OBJ_WRITE_UNLOCK(cmgrp);

    return grps_tobe_freed;
}

PUBLIC void
chunk_manager_destroy (chunk_manager_t *cmgrp)
{
    chunk_group_t *grp, *next_grp;

    OBJ_WRITE_LOCK(cmgrp);
    grp = cmgrp->groups;
    while (grp) {
        next_grp = grp->next_chunk_group;
        MEM_MONITOR_FREE(grp->chunks_block);
        MEM_MONITOR_FREE(grp);
        grp = next_grp;
    }
    memset(cmgrp, 0, sizeof(chunk_manager_t));
}

#ifdef __cplusplus
} // extern C
#endif





