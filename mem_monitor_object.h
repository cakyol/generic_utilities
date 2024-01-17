
/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
** 
** @@@@@ Memory book keeping object
**       Use these instead of malloc/free and it keeps a very accurate
**       count of how much memory has been used, how many times
**       allocation has been called and how many times free has been 
**       called.
**
**       It also zeroes out the allocated memory too.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __MEM_MONITOR_OBJECT_H__
#define __MEM_MONITOR_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include "common.h"

typedef struct mem_monitor_s {

    unsigned long long bytes_used;
    unsigned long long allocations;
    unsigned long long frees;

} mem_monitor_t;

extern void *
mem_monitor_allocate (mem_monitor_t *mmp, int size,
    bool initialize_to_zero);

extern void *
mem_monitor_reallocate (mem_monitor_t *mmp,
    void *ptr, int newsize, bool initialize_to_zero);

extern void
mem_monitor_free (void *ptr);

#define MEM_MON_VARIABLES \
    mem_monitor_t mem_mon, *mem_mon_p

#define MEM_MONITOR_SETUP(objp) \
    do { \
        objp->mem_mon.bytes_used = 0; \
        objp->mem_mon.allocations = 0; \
        objp->mem_mon.frees = 0; \
        objp->mem_mon_p = \
            parent_mem_monitor ? parent_mem_monitor : &objp->mem_mon; \
    } while (0)

#define MEM_MONITOR_ALLOC(objp, size) \
    mem_monitor_allocate(objp->mem_mon_p, size, false)

#define MEM_MONITOR_ZALLOC(objp, size) \
    mem_monitor_allocate(objp->mem_mon_p, size, true)

#define MEM_MONITOR_REALLOC(objp, oldp, newsize) \
    mem_monitor_reallocate(objp->mem_mon_p, oldp, newsize, false)

#define MEM_MONITOR_ZREALLOC(objp, oldp, newsize) \
    mem_monitor_reallocate(objp->mem_mon_p, oldp, newsize, true)

#define MEM_MONITOR_FREE(ptr) \
    if (ptr) mem_monitor_free(ptr)

#define OBJECT_MEMORY_USAGE(objp, size_in_bytes, size_in_megabytes) \
    do { \
        size_in_bytes = ((unsigned long long int) (sizeof(*(objp)) + \
            (objp)->mem_mon_p->bytes_used)); \
        size_in_megabytes = ((double) size_in_bytes / (double) (1024 * 1024)); \
    } while (0)

#ifdef __cplusplus
} // extern C
#endif 

#endif // __MEM_MONITOR_OBJECT_H__

