
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

/******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
**
** This is a generic debug framework which 'reports' bugs/errors based on
** the level allowed for a module.
**
** There are 5 levels of debugging: debug, information, warning, error and
** fatal error in that specific order.  If a specific level is set for a
** module, lower level debugs will not be reported.  The exception to this
** is errors (ERROR, FATAL_ERROR) which will ALWAYS be reported.
** Note that fatal error call will crash the system with an assert call.
** It should be used only as a last resort.
**
** This framework can be made thread safe.  This means a report/message 
** will be completely reported and finished before another thread can be
** executed which may have the potential to interfere in the reporting of
** the message.  This feature can be turned on at initialization time.
**
** Note the word used as 'reporting' rather than 'printing' a message.  This
** is because the user can override the default printing function (which is
** simply writing to stderr) by a user specified one.  It is expected that
** such a user defined function will typically do some sort of printing but it 
** can be anything defined by the user.  Note that another debug/error message
** should NOT  be called inside the user defined function or deadlock will
** result.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __DEBUG_FRAMEWORK_H__
#define __DEBUG_FRAMEWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#include "common.h"
#include "lock_object.h"

/*
 * Debug levels, the higher the number, the higher the priority.
 * (Must fit into an unsigned byte).
 * ERROR_LEVEL & FATAL_ERROR will always gets reported.
 * Critical errors will also always report AND crash the system.
 */
#define TRACE_DEBUG_LEVEL               0
#define INFORM_DEBUG_LEVEL              1
#define WARNING_DEBUG_LEVEL             2
#define ERROR_DEBUG_LEVEL               3
#define FATAL_ERROR_DEBUG_LEVEL         4

typedef struct debug_module_block_s debug_module_block_t;

/* user will completely handle everything */
typedef void (*debug_reporting_function)
    (debug_module_block_t *dmbp, int level,
     const char *file_name, const char *function_name, const int line_number,
     char *fmt, va_list args);

struct debug_module_block_s {

    LOCK_VARIABLES;
    int level;
    char *module_name;
    debug_reporting_function drf;

};

static inline
void debug_module_block_set_level (debug_module_block_t *dmbp, int level)
{
    /* normalize the level */
    if (level > FATAL_ERROR_DEBUG_LEVEL) {
        level = FATAL_ERROR_DEBUG_LEVEL;
    } else if (level < TRACE_DEBUG_LEVEL) {
        level = TRACE_DEBUG_LEVEL;
    }
    dmbp->level = level;
}

static inline void
debug_module_block_set_module_name (debug_module_block_t *dmbp,
        char *name)
{
    dmbp->module_name = name;
}

static inline void
debug_module_block_set_reporting_function (debug_module_block_t *dmbp,
        debug_reporting_function drf)
{
    dmbp->drf = drf;
}

#define TRACE(dmbp, fmt, args...) \
    do { \
        if ((dmbp)->level > TRACE_DEBUG_LEVEL) break; \
        _process_debug_message_(dmbp, TRACE_DEBUG_LEVEL, \
            __FILE__, __FUNCTION__, __LINE__, fmt, ## args); \
    } while (0)

#define INFO(dmbp, fmt, args...) \
    do { \
        if ((dmbp)->level > INFORM_DEBUG_LEVEL) break; \
        _process_debug_message_(dmbp, INFORM_DEBUG_LEVEL, \
            __FILE__, __FUNCTION__, __LINE__, fmt, ## args); \
    } while (0)
    
#define WARN(dmbp, fmt, args...) \
    do { \
        if ((dmbp)->level > WARNING_DEBUG_LEVEL) break; \
        _process_debug_message_(dmbp, WARNING_DEBUG_LEVEL, \
            __FILE__, __FUNCTION__, __LINE__, fmt, ## args); \
    } while (0)
    
/*
 * Below are ALWAYS reported regardless of the level set.
 */

/* errors are ALWAYS reported */
#define ERROR(dmbp, fmt, args...) \
    do { \
        _process_debug_message_(dmbp, ERROR_DEBUG_LEVEL, \
            __FILE__, __FUNCTION__, __LINE__, fmt, ## args); \
    } while (0)

/* fatal errors are ALWAYS reported */
#define FATAL_ERROR(dmbp, fmt, args...) \
    do { \
        _process_debug_message_(dmbp, FATAL_ERROR_DEBUG_LEVEL, \
            __FILE__, __FUNCTION__, __LINE__, fmt, ## args); \
        assert(0); \
    } while (0)

extern int
debug_module_block_init (debug_module_block_t *dmbp,
        bool make_it_thread_safe,
        int level, char *name, debug_reporting_function drf);

/**************************************************************************
 *
 * PRIVATE, DO NOT USE.  DEFINED ONLY TO PASS COMPILATIONS.
 *
 */
extern void
_process_debug_message_ (debug_module_block_t *dmbp, int level,
    const char *file_name, const char *function_name, int line_number,
    char *fmt, ...);

#ifdef __cplusplus
} // extern C
#endif

#endif /* __DEBUG_FRAMEWORK_H__ */



