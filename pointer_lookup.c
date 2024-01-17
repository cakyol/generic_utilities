
#include "common.h"

/*
 * The code below implements a more reliable way of extracting pointers
 * from a structure, in which the value pointed to by the pointer may
 * have accidentally changed.  It catches this condition and returns
 * a NULL.
 *
 * For this scheme to be useful, instead of a raw pointer, a pair of shorts
 * (type 'pointer_representation_t') is used.  One of these is an index
 * into an internal table and the other is a unique monotonically
 * increasing number paired only to this pointer.  It is almost
 * impossible therefore for a pointer to be re-used or changed whose
 * paired number is also corrupted.  The system checks this unique
 * number to ensure that the pointer stored, is the exact pointer
 * which is also requested.
 *
 * The whole thing is managed by the control object 'pointer_lookup_t'
 * in an object oriented manner with very simple interfaces to use.
 *
 * A 'pointer_lookup_t' object must be initialized by the call to
 * 'pointer_lookup_init'.  The parameter is the size of the
 * lookup table to be managed.  Note that since the system
 * can only deal with up to 'MAX_ADDRESS_NUMBER' of pointers,
 * specifying a greater size (or any memory allocation failure),
 * will return an error code.  0 is returned for success.
 *
 * When a pointer needs to be stored anywhere, the function 
 * 'pointer_lookup_store_pointer' is used.  This function returns
 * the index & number of the stored pointer if successfull.
 * In your private structures, instead of the raw pointer, you
 * can now store this index & number instead.
 *
 * When the raw pointer needs to be retreived, 'pointer_lookup_get_pointer'
 * is used, with the same index & number.
 *
 * If the pointer needs to be cleared out/deleted, 'pointer_lookup_clear_pointer'
 * is used.
 *
 * If the entire lookup structure is to be obliterated, 'pointer_lookup_destroy'
 * is used.
 */

/* MUST fit into a signed short */
#define MAX_ADDRESS_NUMBER      32000

/*
 * Must fit into 32 bits.
 *
 * Replaces a raw pointer by an index to a lookup table
 * and a unique number to check the validity of the pointer.
 */
typedef struct pointer_representation_s {

    short index;
    short pointer_number;

} pointer_representation_t;

/*
 * In the internal lookup table, this is how a pointer is represented.
 */
typedef struct pointer_entry_s {

    /* unique number assigned to this specific pointer */
    short pointer_number;

    /* actual pointer to the object in question */
    void *pointer;

} pointer_entry_t; 

/*
 * Control structure which manages all the above
 */
typedef struct pointer_lookup_s {

    /* how many pointers are we handling */
    int size;

    /* unique number which increments & wraps around with each new pointer */
    short number_generator;

    /* the array which holds the actual pointer entries */
    pointer_entry_t *pointer_entries;

    /* stack which contains all the free indexes for the above array */
    short *index_stack;
    int stack_idx;

} pointer_lookup_t;

int
pointer_lookup_init (pointer_lookup_t *plp, int size)
{
    int i;

    if (size >= MAX_ADDRESS_NUMBER) return E2BIG;

    plp->pointer_entries = malloc(size * sizeof(pointer_entry_t));
    if (plp->pointer_entries == NULL) return ENOMEM;

    plp->index_stack = malloc(size * sizeof(short));
    if (plp->index_stack == NULL) {
        free(plp->pointer_entries);
        return ENOMEM;
    }

    plp->size = size;
    plp->stack_idx = 0;
    plp->number_generator = 0;

    for (i = 0; i < size; i++) {

        /* every entry is invalid at start */
        plp->pointer_entries[i].pointer = NULL;
        plp->pointer_entries[i].pointer_number = MAX_ADDRESS_NUMBER;

        /* all table indexes are available at start */
        plp->index_stack[i] = (short) i;
    }

    return 0;
}

static inline unsigned short
pointer_lookup_get_unique_number (pointer_lookup_t *plp)
{
    /* MAX_ADDRESS_NUMBER is represents an invalid entry, skip over it */
    if (plp->number_generator >= MAX_ADDRESS_NUMBER) plp->number_generator = 0;

    return (plp->number_generator)++;
}

static inline int
pointer_lookup_alloc_index (pointer_lookup_t *plp)
{
    /* all array entries are full */
    if (plp->stack_idx >= plp->size) return -1;

    return plp->index_stack[(plp->stack_idx)++];
}

static inline void
pointer_lookup_free_index (pointer_lookup_t *plp, short index)
{
    /* return it back to the stack of free indexes */
    plp->index_stack[--(plp->stack_idx)] = index;
}

int
pointer_lookup_store_pointer (pointer_lookup_t *plp,
    void *user_pointer,
    short *index_returned, short *number_returned)
{
    short index, number;

    /* get a new slot in the pointer table */
    index = pointer_lookup_alloc_index(plp);
    if (index < 0) return ENOSPC;

    /* this always succeeds, cycles thru to the next number */
    number = pointer_lookup_get_unique_number(plp);

    plp->pointer_entries[index].pointer = user_pointer;
    plp->pointer_entries[index].pointer_number = number;

    /* return these to the user */
    *index_returned = index;
    *number_returned = number;

    /* success */
    return 0;
}

void *
pointer_lookup_get_pointer (pointer_lookup_t *plp,
    short index, short number_to_match)
{
    pointer_entry_t *pep;

    //ASSERT((index >= 0) && (index < plp->size));
    pep = &plp->pointer_entries[index];

    /* a valid entry cannot have MAX_ADDRESS_NUMBER as the unique number */
    // if (pep->pointer_number == MAX_ADDRESS_NUMBER) return NULL;

    /* pointer numbers must match */
    if (pep->pointer_number != number_to_match) return NULL;

    /* ok everything passed */
    return pep->pointer;
}

int
pointer_lookup_clear_pointer (pointer_lookup_t *plp,
    short index, short number)
{
    pointer_entry_t *pep;

    //ASSERT((index >= 0) && (index < plp->size));
    pep = &plp->pointer_entries[index];

    if (pep->pointer_number != number) return ENODATA;

    /* ok erase it now */
    pep->pointer = NULL;
    pep->pointer_number = MAX_ADDRESS_NUMBER;

    /* return the array index back to the stack of free indexes */
    pointer_lookup_free_index(plp, index);

    return 0;
}

void
pointer_lookup_destroy (pointer_lookup_t *plp)
{
    free(plp->index_stack);
    free(plp->pointer_entries);
    bzero(plp, sizeof(pointer_lookup_t));
}

#ifdef TESTING

#define MAX     10000

pointer_representation_t ptr_array[MAX];

int main (int argc, char *argv[])
{
    int i, rc;
    pointer_lookup_t pl;
    short index, number;
    void *ptr;

    pointer_lookup_init(&pl, MAX);
    for (i = 0; i < MAX; i++) {
        ptr = integer2pointer(i);
        rc = pointer_lookup_store_pointer(&pl, ptr, &index, &number);
        if (rc) {
            printf("pointer_lookup_store_pointer failed for pointer %d\n", i);
        } else {
            printf("ptr %p stored at index %d number %d\n", ptr, index, number);
            ptr_array[i].index = index;
            ptr_array[i].pointer_number = number;
        }
    }

    for (i = 0; i < MAX; i++) {
        ptr = pointer_lookup_get_pointer(&pl,
                ptr_array[i].index,
                ptr_array[i].pointer_number);
        if (integer2pointer(i) != ptr) {
            printf("mismatch at entry %d, index %d number %d\n", i,
                    ptr_array[i].index, ptr_array[i].pointer_number);
        }

        pointer_lookup_clear_pointer(&pl, ptr_array[i].index, ptr_array[i].pointer_number);
    }

    printf("stack index value = %d\n", pl.stack_idx);


    return 0;

}

#endif /* TESTING */





