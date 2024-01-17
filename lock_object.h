
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

/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
**
** @@@@@
** 
** READ/WRITE LOCKS 
**
** Read/Write synchronizer to be used between processes as
** well as threads.  It can achieve the following:
**
**      - Can be used in shared memory between multiple processes.
**      - No limit on readers (well.. MAXUSHORT).
**      - read locks are recursive but discouraged.
**      - Only one active writer at a time.
**      - write locks are *NOT* recursive, deadlock *WILL* occur if recursive
**        write locking is attempted.
**      - read locks will not starve out a write lock.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __LOCK_OBJECT_H__
#define __LOCK_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>

#include "common.h"
#include "timer_object.h"

typedef struct lock_obj_s {

    /* used with compare & swap, protects rest of the variables */
    volatile byte mtx;

    short readers;
    tinybool write_pending;
    tinybool writing;

} lock_obj_t;

extern int 
lock_obj_init (lock_obj_t *lck);

extern void
grab_read_lock (lock_obj_t *lck);

extern void 
release_read_lock (lock_obj_t *lck);

extern void
grab_write_lock (lock_obj_t *lck);

extern void
release_write_lock (lock_obj_t *lck);

extern void 
lock_obj_destroy (lock_obj_t *lck);

#define LOCK_VARIABLES \
    lock_obj_t lock_structure; \
    lock_obj_t *lock; \

/*
 * Enable locking of the object for anticipated thread use.
 * This can be called dynamically anytime on the object.
 */
#define ENABLE_LOCKING(obj) \
    do { \
        lock_obj_init(&obj->lock_structure); \
        obj->lock = &obj->lock_structure; \
    } while (0)

/*
 * Disable locking of an object (releases all existing locks)
 * This can be called dynamically anytime on the object.
 */
#define DISABLE_LOCKING(obj) \
    do { \
        lock_obj_init(&obj->lock_structure); \
        obj->lock = NULL; \
    } while (0)

/*
 * If locking is required, set up the object's lock structure and let
 * the object's 'lock' pointer point to it.  Otherwise, the pointer
 * is set to NULL (indicating locking is not required).
 */
#define LOCK_SETUP(obj) \
    do { \
        if (make_it_thread_safe) { \
            ENABLE_LOCKING(obj); \
        } else { \
            DISABLE_LOCKING(obj); \
        } \
    } while (0)

#define SAFE_READ_LOCK(lck)         if (lck) grab_read_lock(lck)
#define SAFE_WRITE_LOCK(lck)        if (lck) grab_write_lock(lck)
#define SAFE_READ_UNLOCK(lck)       if (lck) release_read_lock(lck)
#define SAFE_WRITE_UNLOCK(lck)      if (lck) release_write_lock(lck)

#define OBJ_READ_LOCK(obj)          SAFE_READ_LOCK(obj->lock)
#define OBJ_WRITE_LOCK(obj)         SAFE_WRITE_LOCK(obj->lock)
#define OBJ_READ_UNLOCK(obj)        SAFE_READ_UNLOCK(obj->lock)
#define OBJ_WRITE_UNLOCK(obj)       SAFE_WRITE_UNLOCK(obj->lock)

#define LOCK_OBJ_DESTROY(obj) \
    do { \
        if (obj->lock) { \
            lock_obj_destroy(obj->lock); \
            obj->lock = NULL; \
        } \
    } while (0)

#ifdef __cplusplus
} // extern C
#endif 

#endif // __LOCK_OBJECT_H__


