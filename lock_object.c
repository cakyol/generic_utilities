
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

#include "lock_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * equivalent of test and test and set
 */
static inline int
ttas (volatile byte *variable, byte checked, byte set)
{
    return
        (*variable == 0) &&
        (__sync_val_compare_and_swap(variable, checked, set) == 0);
}

/*
 * either yield to another thread or spin lock
 */
static inline void
holdoff (lock_obj_t *lck)
{
#if 1
    SUPPRESS_UNUSED_VARIABLE_COMPILER_WARNING(lck);
    sched_yield();
#else
    if (lck->yield_if_locked) {
        sched_yield();
    } else {
        volatile int i;
        for (i = 0; i < 40000; i++);
    }
#endif
}

#define PUBLIC

/******* Public functions start here *****************************************/

PUBLIC int 
lock_obj_init (lock_obj_t *lck /*, int yield_if_locked */)
{
    memset((void*) lck, 0, sizeof(lock_obj_t));
    return 0;
}

static int 
_read_lock (lock_obj_t *lck, boolean block)
{
    while (1) {
        if (ttas(&lck->mtx, 0, 1)) {
            if (!lck->write_pending) {
                lck->readers++;
                lck->mtx = 0;
                return 0;
            }
            lck->mtx = 0;
        }
        if (block) {
            holdoff(lck);
        } else {
            return -1;
        }
    }
}

PUBLIC void
grab_read_lock (lock_obj_t *lck)
{ (void) _read_lock(lck, true); }

PUBLIC void 
release_read_lock (lock_obj_t *lck)
{
    while (1) {
        if (ttas(&lck->mtx, 0, 1)) {
            if (--lck->readers < 0) lck->readers = 0;
            lck->mtx = 0;
            return;
        }
        holdoff(lck);
    }
}

static int
_write_lock (lock_obj_t *lck, boolean block)
{
    while (1) {
        if (ttas(&lck->mtx, 0, 1)) {

            /*
             * dont clear this since 'grab_read_lock'
             * will check it
             */
            lck->write_pending = true;

            if ((lck->readers <= 0) && (!lck->writing)) {
                lck->writing = true;
                lck->mtx = 0;
                return 0;
            }
            lck->mtx = 0;
        }
        if (block) {
            holdoff(lck);
        } else {
            return -1;
        }
    }
}

PUBLIC void
grab_write_lock (lock_obj_t *lck)
{ return (void) _write_lock(lck, true); }

PUBLIC void 
release_write_lock (lock_obj_t *lck)
{
    while (1) {
        if (ttas(&lck->mtx, 0, 1)) {
            lck->write_pending = lck->writing = false;
            lck->mtx = 0;
            return;
        }
        holdoff(lck);
    }
}

void
lock_obj_destroy (lock_obj_t *lck)
{
    if (lck) memset(lck, 0, sizeof(lock_obj_t));
}

#ifdef __cplusplus
} // extern C
#endif 



