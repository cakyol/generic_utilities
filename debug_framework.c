
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

#include "debug_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char *level_strings [] = { 
    [TRACE_DEBUG_LEVEL] = "TRACE",
    [INFORM_DEBUG_LEVEL] = "INFORMATION",
    [WARNING_DEBUG_LEVEL] = "WARNING",
    [ERROR_DEBUG_LEVEL] = "ERROR",
    [FATAL_ERROR_DEBUG_LEVEL] = "FATAL_ERROR" };

/*
 * When this function is called, both the module number
 * and level are already verified (thru the macros calling it)
 * so range check does not need to be done again.
 */
void
_process_debug_message_ (debug_module_block_t *dmbp, int level,
    const char *file_name, const char *function_name, const int line_number,
    char *fmt, ...)
{
    va_list args;

    /*
     * In a multi threaded environment, we dont want the
     * printing/reporting to be garbled if context
     * switching occurs while in the middle of printing
     * an error string.
     */
    OBJ_WRITE_LOCK(dmbp);

    va_start(args, fmt);
    if (dmbp->drf) {
        dmbp->drf(dmbp, level, file_name, function_name,
            line_number, fmt, args);
    } else {
        fprintf(stderr, "%s: %s: %s(%d): <%s>: ",
            level_strings[level], &dmbp->module_name[0],
            file_name, line_number, function_name);
        vfprintf(stderr, fmt, args);
        fflush(stderr);
    }

    OBJ_WRITE_UNLOCK(dmbp);
}

PUBLIC int
debug_module_block_init (debug_module_block_t *dmbp,
        boolean make_it_thread_safe,
        int level, char *name, debug_reporting_function drf)
{
    LOCK_SETUP(dmbp);
    debug_module_block_set_level(dmbp, level);
    debug_module_block_set_module_name(dmbp, name);
    debug_module_block_set_reporting_function(dmbp, drf);

    return 0;
}

#ifdef __cplusplus
} // extern C
#endif



