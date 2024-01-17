
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

#include <limits.h>
#include "bitlist_object.h"

#if INT_MAX == 32767
    #define BITS_PER_INT                16
    #define BYTES_PER_INT               2
    #define BITS_TO_INT_SHIFT           4
    #define ALL_ONES                    0xFFFF
#elif INT_MAX == 2147483647
    #define BITS_PER_INT                32
    #define BYTES_PER_INT               4
    #define BITS_TO_INT_SHIFT           5
    #define ALL_ONES                    0xFFFFFFFF
#elif INT_MAX == 9223372036854775807
    #define BITS_PER_INT                64
    #define BYTES_PER_INT               8       
    #define BITS_TO_INT_SHIFT           6
    #define ALL_ONES                    0xFFFFFFFFFFFFFFFF
#else
    #error "What kind of weird system are you on?"
#endif
#define MAX_BIT_NUMBER                  (BITS_PER_INT - 1)

#define PUBLIC

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The "%" operator, much faster to do it like this
 */
#define MODULO(x) \
    ((x) - (((x) >> BITS_TO_INT_SHIFT) << BITS_TO_INT_SHIFT))

#define BIT_GET(ints, bit) \
    ((ints[bit >> BITS_TO_INT_SHIFT]) & (1 << MODULO(bit)))

#define BIT_SET(ints, bit) \
    ints[bit >> BITS_TO_INT_SHIFT] |= (1 << MODULO(bit))

#define BIT_CLEAR(ints, bit) \
    ints[bit >> BITS_TO_INT_SHIFT] &= ~(1 << MODULO(bit))

static inline int
_first_clear_bit (unsigned int value)
{
    int i;

    for (i = 0; i < BITS_PER_INT; i++) {
        if (0 == (value & (1 << i))) return i;
    }
    return -1;
}

static int
thread_unsafe_bitlist_get (bitlist_t *bl, int bit_number, int *returned_bit)
{
    unsigned int value;

    /* check bounds */
    if (bit_number < bl->lowest_valid_bit) return EINVAL;
    if (bit_number > bl->highest_valid_bit) return EINVAL;

    /* adjust offset */
    bit_number -= bl->lowest_valid_bit;

    /* get & return it */
    value = BIT_GET(bl->the_bits, bit_number);
    *returned_bit = value ? 1 : 0;

    /* no error */
    return 0;
}

static int
thread_unsafe_bitlist_set (bitlist_t *bl, int bit_number)
{
    unsigned int bit;

    /* check bounds */
    if (bit_number < bl->lowest_valid_bit) return EINVAL;
    if (bit_number > bl->highest_valid_bit) return EINVAL;

    /* adjust offset */
    bit_number -= bl->lowest_valid_bit;

    /* set it */
    bit = BIT_GET(bl->the_bits, bit_number);
    if (bit == 0) {
        BIT_SET(bl->the_bits, bit_number);
        bl->bits_set_count++;
    }

    /* no error */
    return 0;
}

static int
thread_unsafe_bitlist_clear (bitlist_t *bl, int bit_number)
{
    unsigned int value;

    /* check bounds */
    if (bit_number < bl->lowest_valid_bit) return EINVAL;
    if (bit_number > bl->highest_valid_bit) return EINVAL;

    /* adjust offset */
    bit_number -= bl->lowest_valid_bit;

    /* clear it */
    value = BIT_GET(bl->the_bits, bit_number);
    if (value) {
        BIT_CLEAR(bl->the_bits, bit_number);
        bl->bits_set_count--;
    }

    /* no error */
    return 0;
}

static int
thread_unsafe_bitlist_first_set_bit (bitlist_t *bl, int *returned_bit_number)
{
    int i, first;

    for (i = 0; i < bl->size_in_ints; i++) {
        if (bl->the_bits[i]) {
            first = ffs(bl->the_bits[i]);
            first = (first - 1) + (i * BITS_PER_INT) + bl->lowest_valid_bit;
            if (first > bl->highest_valid_bit) return ENODATA;
            *returned_bit_number = first;
            return 0;
        }
    }
    return ENODATA;
}

static int
thread_unsafe_bitlist_first_clear_bit (bitlist_t *bl, int *returned_bit_number)
{
    int i, first;

    for (i = 0; i < bl->size_in_ints; i++) {
        if (bl->the_bits[i] != ALL_ONES) {
            first = _first_clear_bit(bl->the_bits[i]);
            first += (i * BITS_PER_INT) + bl->lowest_valid_bit;
            if (first > bl->highest_valid_bit) return ENODATA;
            *returned_bit_number = first;
            return 0;
        }
    }
    return ENODATA;
}


/******* Public functions ****************************************************/

PUBLIC int
bitlist_init (bitlist_t *bl,
    int make_it_thread_safe,
    int lowest_valid_bit, int highest_valid_bit,
    int initialize_to_all_ones,
    mem_monitor_t *parent_mem_monitor)
{
    /* allow some extra at the end so we can run thru using complete ints */
    int size_in_ints = 
        ((highest_valid_bit - lowest_valid_bit + 1) / BITS_PER_INT) + 2;
    int i, failed = 0;
    unsigned int data;

    MEM_MONITOR_SETUP(bl);
    LOCK_SETUP(bl);

    bl->the_bits = 
        (unsigned int*) MEM_MONITOR_ZALLOC(bl, (size_in_ints * BYTES_PER_INT));
    if (0 == bl->the_bits) {
        failed = ENOMEM;
        goto done;
    }
    bl->size_in_ints = size_in_ints;
    bl->lowest_valid_bit = lowest_valid_bit;
    bl->highest_valid_bit = highest_valid_bit;
    if (initialize_to_all_ones) {
        data = ALL_ONES;
        bl->bits_set_count = highest_valid_bit - lowest_valid_bit + 1;
    } else {
        data = 0;
        bl->bits_set_count = 0;
    }
    for (i = 0; i < size_in_ints; i++) bl->the_bits[i] = data;
done:
    return failed;
}

PUBLIC int
bitlist_get (bitlist_t *bl, int bit_number, int *returned_bit)
{
    int failed;

    OBJ_READ_LOCK(bl);
    failed = thread_unsafe_bitlist_get(bl, bit_number, returned_bit);
    OBJ_READ_UNLOCK(bl);
    return failed;
}

PUBLIC int
bitlist_set (bitlist_t *bl, int bit_number)
{
    int failed;

    OBJ_WRITE_LOCK(bl);
    failed = thread_unsafe_bitlist_set(bl, bit_number);
    OBJ_WRITE_UNLOCK(bl);
    return failed;
}

PUBLIC int
bitlist_clear (bitlist_t *bl, int bit_number)
{
    int failed;

    OBJ_WRITE_LOCK(bl);
    failed = thread_unsafe_bitlist_clear(bl, bit_number);
    OBJ_WRITE_UNLOCK(bl);
    return failed;
}

PUBLIC int
bitlist_first_set_bit (bitlist_t *bl, int *returned_bit_number)
{
    int failed;

    OBJ_READ_LOCK(bl);
    failed = thread_unsafe_bitlist_first_set_bit(bl, returned_bit_number);
    OBJ_READ_UNLOCK(bl);
    return failed;
}

PUBLIC int
bitlist_first_clear_bit (bitlist_t *bl, int *returned_bit_number)
{
    int failed;

    OBJ_READ_LOCK(bl);
    failed = thread_unsafe_bitlist_first_clear_bit(bl, returned_bit_number);
    OBJ_READ_UNLOCK(bl);
    return failed;
}

/*
 * put all the sanity checks in this function.
 */
static bool
error_in_bit_numbers (int start, int size,
    bool check_value, uint64_t value)
{
    return false;
}

PUBLIC int
get_bit_group (uint64_t data,
    byte start, byte size, bool check,
    uint64_t *raw, uint64_t *normalized)
{
    int sr = start - size + 1;
    int sl = MAX_BIT_NUMBER - start;
    uint64_t new;

    if (check) {
        if (error_in_bit_numbers(start, size, false, 0)) {
            return EINVAL;
        }
    }

    new = (data >> sr) << sr;
    new = (new << sl) >> sl;
    safe_pointer_set(raw, new);
    safe_pointer_set(normalized, (new >> sr));

    return 0;
}

PUBLIC int
set_bit_group (uint64_t *data,
    byte start, byte size, uint64_t value, bool check)
{
    uint64_t left, right;
    int ls, rs;

    if (check) {
        if (error_in_bit_numbers(start, size, true, value)) {
            return EINVAL;
        }
    }

    ls = start + 1;
    rs = MAX_BIT_NUMBER - (start - size);
    left = (*data >> ls) << ls;
    right = (*data << rs) >> rs;

    /* FIX THIS */
    *data = left | right | (value << start);

    return 0;
}

PUBLIC void
bitlist_destroy (bitlist_t *bl)
{
    OBJ_WRITE_LOCK(bl);
    if (bl->the_bits) MEM_MONITOR_FREE(bl->the_bits);
    OBJ_WRITE_UNLOCK(bl);
    LOCK_OBJ_DESTROY(bl);
    memset(bl, 0, sizeof(bitlist_t));

}

#ifdef __cplusplus
} // extern C
#endif





