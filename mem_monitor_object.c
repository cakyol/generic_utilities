/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
** 
** @@@@@ Memory accounting object
**       Keeps a very accurate measurement of how much dynamic
**       memory has been used, how many times the alloc has been
**       invoked and how many times the free has been invoked.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "mem_monitor_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * All we need to process memory requests.  It is a cheat structure
 * inserted in front of the memory returned to user.  
 * It has to be 8 bytes aligned so that it works with all
 * alignments hence the use of 'unsigned long long data [0]'.
 */
typedef struct mem_header_s {

    /* the relevant mem monitor */
    mem_monitor_t *mmp;

    /* total size of bytes used INCLUDING THIS header */
    int total_size;

    /* make the whole size of the structure a mult of 8 bytes */
    unsigned long long data [0];

} mem_header_t;

/*
 * extract the mem_header_t given the user pointer
 */
static inline mem_header_t *
get_mem_header_ptr (void *ptr)
{
    return
        (mem_header_t*) (((byte*) ptr) - sizeof(mem_header_t));
}

/*
 * An extra mem_header_t is inserted into the front
 * of all memory returrned to the user so we have all
 * the necessary information when the pointer gets freed.
 */
void *
mem_monitor_allocate (mem_monitor_t *mmp,
        int size, bool initialize_to_zero)
{
    int total_size = size + sizeof(mem_header_t);
    mem_header_t *mhp;
    byte *block;

    block = malloc(total_size);
    if (block) {
        if (initialize_to_zero) memset(block, 0, total_size);
        mhp = (mem_header_t*) block;
        mhp->mmp = mmp;
        mhp->total_size = total_size;
        if (mmp) {
            mmp->bytes_used += total_size;
            mmp->allocations++;
        }
        return &(mhp->data[0]);
    }
    return null;
}

void
mem_monitor_free (void *ptr)
{
    mem_header_t *mhp;

    mhp = get_mem_header_ptr(ptr);
    if (mhp->mmp) {
        mhp->mmp->bytes_used -= mhp->total_size;
        mhp->mmp->frees++;
    }

    free(mhp);
}

void *
mem_monitor_reallocate (mem_monitor_t *mmp,
    void *ptr, int new_data_size,
    bool initialize_to_zero)
{
    mem_header_t *mhp;
    int old_total_size, new_total_size;
    byte *new_data;

    /* for a null pointer, this just becomes a new alloc */
    if (NULL == ptr) {
        return
            mem_monitor_allocate(mmp, new_data_size, initialize_to_zero);
    }

    /* record old stuff */
    mhp = get_mem_header_ptr(ptr);
    assert(mmp == mhp->mmp);
    old_total_size = mhp->total_size;
    new_total_size = new_data_size + sizeof(mem_header_t);

    /* get new memory */
    new_data = realloc(mhp, new_total_size);
    if (new_data) {
        if (initialize_to_zero) memset(mhp, 0, new_total_size);
        mmp->bytes_used -= old_total_size;
        mmp->bytes_used += new_total_size;
        mhp = (mem_header_t*) new_data;
        mhp->mmp = mmp;
        mhp->total_size = new_total_size;
        return &(mhp->data[0]);
    }

    /* if we are here, realloc failed, nothing we can do */
    return null;
}

#ifdef __cplusplus
} // extern C
#endif 


