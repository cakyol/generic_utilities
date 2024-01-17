
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
** Bit list/map object, FIRST BIT IS ALWAYS 0.
**
** All answers are adjusted to the offset
** Return values are 0 for success or an errorcode, except
** for the functions which return a position as value.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __BITLIST_OBJECT_H__
#define __BITLIST_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdlib.h>
#include <strings.h>

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"

typedef struct bitlist_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    int lowest_valid_bit;
    int highest_valid_bit;
    int size_in_ints;
    int bits_set_count;
    unsigned int *the_bits;

} bitlist_t;

static inline int
bitlist_count_ones (bitlist_t *bl)
{ return bl->bits_set_count; }

static inline int
bitlist_count_zeros (bitlist_t *bl)
{
    return 
        bl->highest_valid_bit - bl->lowest_valid_bit + 1 - 
        bl->bits_set_count; 
}

extern int
bitlist_init (bitlist_t *bl,
    int make_it_thread_safe,
    int lowest_valid_bit, int highest_valid_bit,
    int initialize_to_all_ones,
    mem_monitor_t *parent_mem_monitor);

extern int
bitlist_get (bitlist_t *bl, int bit_number, int *returned_bit);

extern int
bitlist_set (bitlist_t *bl, int bit_number);

extern int
bitlist_clear (bitlist_t *bl, int bit_number);

extern int
bitlist_first_set_bit (bitlist_t *bl, int *returned_bit_number);

extern int
bitlist_first_clear_bit (bitlist_t *bl, int *returned_bit_number);

extern void
bitlist_destroy (bitlist_t *bl);

/*
 * Extracts the value of bits from 'data', starting from bit number 'start'
 * extending 'size' number of bits.  The returned value is placed into
 * two separate uint64_t.  The 'raw' value is the raw bits in the exact
 * position they were in.  The 'normalized' value is the returned bits
 * shifted to the right as much as possible so they can be accessed as
 * a number starting from 0.  Both 'raw' and 'normalized' CAN be NULL
 * altho calling this function then would be pointless.
 *
 * REMEMBER ALL BIT NUMBERS START FROM ZERO (0) LSB.
 *
 * For example if we want to return 4 bits
 * starting at bit number 6 from the bit pattern below:
 *
 *      .......1011110011100111
 *                      ^^^^^^^
 *                      6543210 (bit numbers)
 *
 *  In 'raw', you would have .....0000000 1100 000.
 *                                        ^^^^
 *  And in 'normalized, you would have .....00000 1100.
 *                                                ^^^^
 * Return value is 0 if everything is ok and errno if the parameters do
 * not make sense, for example asking for the 80 th bit when at most
 * the maximum can be only be 63.
 */
extern int
get_bit_group (uint64_t data,
    byte start, byte size, bool check,
    uint64_t *raw, uint64_t *normalized);

/*
 * Similar to above but places the specified bit pattern (value) into
 * bit position starting at 'start' up to 'size' bits, into 'data'.
 * Return value is the same.  Note that if the vlaue specified does
 * not fint into 'size' bits, it will be an error.
 */
extern int
set_bit_group (uint64_t *data,
    byte start, byte size, uint64_t value, bool check);

#ifdef __cplusplus
} // extern C
#endif

#endif // __BITLIST_OBJECT_H__




