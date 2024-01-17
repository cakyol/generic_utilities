
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

#include "buffer_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

static debug_module_block_t buffer_manager_debug = {
    .lock = NULL,
    .level = ERROR_DEBUG_LEVEL,
    .module_name = "BUFFER_MANAGER_MODULE",
    .drf = NULL
};

/*
 * initialize a buffer pool with all its buffers.  Each buffer can hold
 * data of size 'size' and there will be 'count' of these buffers
 * in this pool.
 *
 * Note that the sanity of the numbers have all been verified
 * by the time the flow reaches here.  So, there is no need to
 * error check anything here.
 *
 * 0 will be returned if successful, else an error code.
 * The only error which can be returned at this stage is if
 * malloc fails, which will be ENOMEM.
 */
static int
buffer_manager_pool_init (buffer_manager_t *bmp,
        int pool_number, size_count_tuple_t *tuple)
{
    int size = tuple->size;
    int count = tuple->count;
    int i, total_size, aligned_data_size, actual_buffer_size;
    byte *block, *ptr;
    buffer_t *bufp = NULL;  /* shut the compiler up */
    buffer_pool_t *poolp = &bmp->pools[pool_number];

    /* empty the pool first */
    memset(poolp, 0, sizeof(buffer_pool_t));

    /* adjusted sizes */
    aligned_data_size = (size + 7) & ~7;
    actual_buffer_size = aligned_data_size + sizeof(buffer_t);
    total_size = actual_buffer_size * count;

    /* allocate the big block for the entire pool and fill fields */
    block = MEM_MONITOR_ALLOC(bmp, total_size);
    
    /* cannot proceed if so */
    if (NULL == block) return ENOMEM;

    /* we are ok, fill the rest */
    poolp->bmp = bmp;
    poolp->specified_size = size;
    poolp->actual_buffer_size = actual_buffer_size;
    poolp->buffer_count = count;
    poolp->block = block;
    poolp->head = (buffer_t*) block;

    /* now partition each buffer in the big block */
    ptr = block;
    for (i = 0; i < count; i++) {
        bufp = (buffer_t*) ptr;
        bufp->poolp = poolp;
        ptr += actual_buffer_size;
        bufp->next = (buffer_t*) ptr;
    }

    /* last buffer */
    bufp->next = NULL;

    return 0;
}

static int
buffer_manager_lookup_table_init (buffer_manager_t *bmp)
{
    int p, idx;

    bmp->size_lookup_table = MEM_MONITOR_ALLOC(bmp, bmp->max_size);
    if (NULL == bmp->size_lookup_table) {
        ERROR(&buffer_manager_debug,
            "allocating %d bytes for buffer manager size lookup array failed\n",
            bmp->max_size);
        return ENOMEM;
    }

    idx = 0;
    for (p = 0; p < bmp->num_pools; p++) {
        for (; idx <= bmp->pools[p].specified_size; idx++) {
            bmp->size_lookup_table[idx] = p;
        }
    }

    return 0;
}

PUBLIC int
buffer_manager_initialize (buffer_manager_t *bmp,
        boolean make_it_thread_safe,
        size_count_tuple_t tuples [],
        mem_monitor_t *parent_mem_monitor)
{
    int p, pcnt, prev_size;

    /*
     * Make one pass over the tuples array, checking everything
     * and making sure that all the numbers are sane & acceptable.
     */
    prev_size = pcnt = 0;
    while (true) {

        int cnt = tuples[pcnt].count;
        int sz = tuples[pcnt].size;

        INFO(&buffer_manager_debug,
            "processing %d buffer pools with requested size of %d bytes\n",
            cnt, sz);
        
        /*
         * if end of tuples is reached (BOTH sz & cnt are negative), we are done
         */
        if ((sz < 0) && (cnt < 0)) {

            /* there must be at least one pool defined */
            if (pcnt == 0) {
                ERROR(&buffer_manager_debug,
                    "buffer manager has no pools and/or buffers defined\n");
                return EINVAL;
            }

            /* normal end */
            break;
        }

        /* too many pools */
        if (pcnt >= MAX_POOLS) {
            ERROR(&buffer_manager_debug,
                "too many pools, max allowed is %d\n", MAX_POOLS);
            return EINVAL;
        }

        /* the number of buffers in a pool must be > 0 */
        if (cnt <= 0) {
            ERROR(&buffer_manager_debug,
                "pool %d number of buffers (%d) must be > 0\n",
                    pcnt, cnt);
            return EINVAL;
        }

        /* buffer size (of any pool) must be > 0 */
        if (sz <= 0) {
            ERROR(&buffer_manager_debug,
                "pool %d buffer size (%d) must be > 0\n", pcnt, sz);
            return EINVAL;
        }

        /* successive buffer sizes of consecutive pools MUST increase */
        if (sz <= prev_size) {
            ERROR(&buffer_manager_debug,
                "pool %d buffer size (%d) must be > the the previous "
                "pool (%d) buffer size (%d)\n",
                pcnt, sz, pcnt-1, prev_size);
            return EINVAL;
        }

        prev_size = sz;
        pcnt++;
    }


    /*
     * the tuples are all sane, we can now proceed with everything else
     */
    MEM_MONITOR_SETUP(bmp);
    LOCK_SETUP(bmp);

    /* first nullify all pools */
    memset(&bmp->pools[0], 0, sizeof(bmp->pools));

    /* set the total number of available pools */
    bmp->num_pools = pcnt;

    /*
     * set the buffer size of the pool with the largest buffer size, 
     * which will always be the last pool processed since the buffer
     * sizes of each consecutive pool are increasing.  What this means is
     * that if a buffer allocation request of size > than this number
     * is required, we cannot honor it.
     */
    bmp->max_size = tuples[pcnt -1].size;

    /* now initialize all the pools and their buffers */
    for (p = 0; p < pcnt; p++) {
        buffer_manager_pool_init(bmp, p, &tuples[p]);
    }

    /* now initialize the size -> pool lookup table for fast allocation */
    buffer_manager_lookup_table_init(bmp);

    OBJ_WRITE_UNLOCK(bmp);

    return 0;
}

PUBLIC void*
buffer_allocate (buffer_manager_t *bmp, int size)
{
    int p;
    buffer_pool_t *poolp;
    buffer_t *bufp;
    void *data = NULL;

    /* no such buffers */
    if ((size < 0) || (size > bmp->max_size)) {
        return NULL;
    }

    OBJ_WRITE_LOCK(bmp);

    /*
     * pools are ordered based on size so that if a
     * particular sized pool is exhausted, a buffer is
     * allocated from the next higher sized pool.
     * We normally start from the pool given to us from the
     * lookup table.  However, if the lookup table was not
     * allocated (usually due to malloc failure), then we
     * start from the first pool (0).
     */
    p = (bmp->size_lookup_table) ? bmp->size_lookup_table[size] : 0;
    while (p < bmp->num_pools) {
        poolp = &bmp->pools[p];
        if ((size <= poolp->specified_size) && poolp->head) {
            bufp = poolp->head;
            poolp->head = bufp->next;
            data = &bufp->data[0];
            break;
        }
        p++;
    }

    OBJ_WRITE_UNLOCK(bmp);
    return data;
}

PUBLIC void
buffer_free (void *ptr)
{
    byte *bptr = ((byte*) ptr) - (int) (sizeof(buffer_t));
    buffer_t *bufp = (buffer_t*) bptr;

    OBJ_WRITE_LOCK(bufp->poolp->bmp);
    bufp->next = bufp->poolp->head;
    bufp->poolp->head = bufp;
    OBJ_WRITE_UNLOCK(bufp->poolp->bmp);
}

PUBLIC void
buffer_manager_destroy (buffer_manager_t *bmp)
{
    int i;
    buffer_pool_t *pools = (buffer_pool_t*) bmp->pools;

    OBJ_WRITE_LOCK(bmp);
    for (i = 0; i < bmp->num_pools; i++) {
        MEM_MONITOR_FREE(pools[i].block);
        memset(&pools[i].block, 0, sizeof(buffer_pool_t));
    }
    MEM_MONITOR_FREE(bmp->size_lookup_table);
    OBJ_WRITE_UNLOCK(bmp);
    LOCK_OBJ_DESTROY(bmp);
    memset(bmp, 0, sizeof(*bmp));
}

#ifdef __cplusplus
} // extern C
#endif


