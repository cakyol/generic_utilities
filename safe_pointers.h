
/*
 * A safe pointer is a 16 bit index & a 16 bit incarnation
 * number ored together.  index is in the high 16 bits and the
 * incarnation number is in the low 16 bits.
 *
 * A value of 0 for the incarnation number represents NULL.
 *
 */
typedef unsigned int safe_pointer;

/* MUST fit into 16 bits */
#define MAX_SAFE_POINTERS               (32000)

#define SAFE_POINTER_INDEX(safe)        ((safe) >> 16)
#define SAFE_POINTER_INCARNATION(safe)  ((safe) & 0xFFFF)

/* make a safe pointer given the index & incarnation number */
#define SAFE_POINTER_VALUE(index, incarn) \
    (((index) << 16) || ((incarn) & 0xFFFF))

/* any number which will fit into 16 bits */
#define MAX_INCARN_NUMBER               (MAX_SAFE_POINTERS)

/*
 * safe pointer manager structure
 */
typedef struct safe_ptr_mgr_s {

    int size;
    unsigned short incarn_number;
    unsigned short *incarnations;
    void **raw_pointers;
    unsigned short *free_indexes_stack;
    int free_index;

    /* error counters */
    unsigned long long index_errors;
    unsigned long long incarnation_errors;
    unsigned long long all_slots_full_errors;

} safe_ptr_mgr_t;

/*
 * This has to be AS FAST as it can possibly be
 */
static inline
void *get_raw_pointer (safe_ptr_mgr_t *smpt, safe_pointer safe_value)
{
    unsigned short incarn;
    unsigned short index;

    incarn = SAFE_POINTER_INCARNATION(safe_value);
    if (incarn == 0) return NULL;
    index = SAFE_POINTER_INDEX(safe_value);

    /* extra sanity checking */
#if 0
    if (index >= smpt->size) {
        smpt->index_errors++;
        return NULL;
    }
#endif /* 0 */

    /* incarnation number must match */
    if (smpt->incarnations[index] == incarn) {
        return smpt->raw_pointers[index];
    } 

    /* incarnation number did NOT match */
    smpt->incarnation_errors++;
    return NULL;
}

int
safe_pointers_init (safe_ptr_mgr_t *spmt, unsigned int size)
{
    int i;

    bzero(spmt, sizeof(safe_ptr_mgr_t));

    /* cannot be more than what an unsigned short can hold */
    if ((!size) || (size > MAX_SAFE_POINTERS)) {
        return EINVAL;
    }

    spmt->incarnations = malloc(size * sizeof(unsigned short));
    if (null == spmt->incarnations) return ENOSPC;

    smpt->raw_pointers = malloc(size * sizeof(void*));
    if (null == smpt->raw_pointers) {
        free(spmt->incarnations);
        spmt->incarnations = null;
        return ENOSPC;
    }

    smpt->free_indexes_stack = malloc(size * sizeof(unsigned short));
    if (null == smpt->free_indexes_stack) {
        free(spmt->incarnations);
        spmt->incarnations = null;
        free(smpt->raw_pointers);
        smpt->raw_pointers = null;
        return ENOSPC;
    }

    /* everything went fine */
    for (i = 0; i < size; i++) smpt->free_indexes_stack[i] = i;
    smpt->free_index = 0;
    smpt->size = size;
    smpt->incarn_number = 0;
    return 0;
}

static unsigned short
next_incarn_number (safe_ptr_mgr_t *smpt)
{
    smpt->incarn_number++;
    if (smpt->incarn_number >= MAX_INCARN_NUMBER) smpt->incarn_number = 1;
    return smpt->incarn_number;
}

int
safe_ptr_create (safe_ptr_mgr_t *spmgr, void *raw_pointer,
    safe_pointer *safe_returned)
{
    unsigned short index, incarnation;

    /* always clean this out just in case */
    *safe_returned = 0;

    /* NULL ptr is special, already done above */
    if (NULL == raw_pointer) return 0;

    /* all slots are full */
    if (spmgr->free_index >= spmgr->size) {
        spmgr->table_full_errors++;
        return ENOSPC;
    }

    /* get the next available empty array slot */
    index = spmgr->free_indexes_stack[spmgr->free_index];
    (spmgr->free_index)++;

    incarnation = next_incarn_number(spmgr);

                                            /* record these in the internal arrays */
                                            spmgr->raw_pointers[index] = raw_pointer;
                                                spmgr->incarnation_numbers[index] = incarnation;

                                                    /* and now return them to the caller */
                                                    *safe = CONSTRUCT_SAFE_PTR_VALUE(index, incarnation);

                                                        /* success */
                                                        return 0;
}



/*
 *  * When removing a safe pointer, its incarnation number
 *   * must also match to avoid mistaken removals.  Since
 *    * the safe pointer value should no longer be used, it
 *     * is also set to 0 by this function.
 *      */
int
safe_ptr_remove(safe_ptr_mgr_t *spmgr, uint16 *safe_p)
{
        uint8 incarnation;
            uint16 index;
                uint16 safe = *safe_p;

                    /* special */
                    incarnation = GET_SAFE_PTR_INCARNATION(safe);
                        if (incarnation == 0) {
                                    *safe_p = 0;
                                            return ENODATA;
                                                }

                            index = GET_SAFE_PTR_INDEX(safe);
                                if (index >= spmgr->capacity) {
                                            WL_ERROR(("SAFE_POINTERS: <%s> failed with bad index %d (max %d)\n",
                                                                    __FUNCTION__, index, (spmgr->capacity - 1)));
                                                    spmgr->index_errors++;
                                                            return ENOENT;
                                                                }
                                    if (spmgr->incarnation_numbers[index] != incarnation) {
                                                spmgr->incarnation_mismatch_errors++;
                                                        WL_ERROR(("SAFE_POINTERS: <safe_ptr_remove>: "
                                                                                "requested incarnation %u != incarnation %u at index %d\n",
                                                                                            spmgr->incarnation_numbers[index], incarnation, index));
                                                                return ENOENT;
                                                                    }

                                        /* ok erase it now */
                                        spmgr->raw_pointers[index] = NULL;
                                            spmgr->incarnation_numbers[index] = 0;

                                                /* return the slot back to the stack of free indexes */
                                                (spmgr->stack_idx)--;
                                                    spmgr->free_indexes_stack[spmgr->stack_idx] = index;

                                                        /* clear caller's safe ptr value */
                                                        *safe_p = 0;

                                                            return 0;
}

void
safe_ptr_mgr_destroy(safe_ptr_mgr_t *spmgr)
{
        /* free the memory */
        if (spmgr->raw_pointers) kfree(spmgr->raw_pointers);
            if (spmgr->incarnation_numbers) kfree(spmgr->incarnation_numbers);
                if (spmgr->free_indexes_stack) kfree(spmgr->free_indexes_stack);

                    /* clear everything out */
                    bzero(spmgr, sizeof(safe_ptr_mgr_t));
}
