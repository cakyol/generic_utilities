
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

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "ez_sprintf.h"

void
ez_sprintf_reset(ez_sprintf_t *ezsp)
{
	ezsp->write_idx = 0;
	ezsp->remaining_size = ezsp->max_size;
	ezsp->current_indent = 0;
}

void
ez_sprintf_init_with_external_buffer(ez_sprintf_t *ezsp,
	char *externally_supplied_buffer, int external_buffer_size,
	int indent_size)
{
	ezsp->buffer = externally_supplied_buffer;
	ezsp->max_size = external_buffer_size - 8;
	ezsp->indent_size = indent_size;
	ez_sprintf_reset(ezsp);
}

/*
 * writes into the sprintf buffer starting from the current write index and
 * updates the current write index and how many bytes still remain to write
 * again.  This function does all the bookkeeping and null terminates the string.
 * The ezsp structure is ready to pass into another ez_sprintf_append if user wants
 * to continue writing.
 *
 * Returns 0 upon success and an error code (ENOSPC) if not enuf space remains.
 */
int
ez_sprintf_append (ez_sprintf_t *ezsp, char *fmt, ...)
{
	va_list args;
	int len, rv = 0;

	/* process the parameters into the buffer */
	va_start(args, fmt);
	len = vsnprintf(&ezsp->buffer[ezsp->write_idx], ezsp->remaining_size,
			fmt, args);
	va_end(args);

	/* update all the counts */
	ezsp->write_idx += len;
	ezsp->remaining_size -= len;
	if (ezsp->remaining_size < 0) {
		ezsp->remaining_size = 0;
		rv = ENOSPC;
	}

	/* always null terminate */
	ezsp->buffer[ezsp->write_idx] = 0;

	return rv;
}

/*
 * output the current indent
 */
int ez_sprintf_indent(ez_sprintf_t *ezsp)
{
	int i;

	if (ezsp->remaining_size < ezsp->current_indent) return ENOSPC;
	for (i = 0; i < ezsp->current_indent; i++) {
		ezsp->buffer[ezsp->write_idx++] = ' ';
	}
	ezsp->remaining_size -= ezsp->current_indent;

	/* always null terminate */
	ezsp->buffer[ezsp->write_idx] = 0;

	return 0;
}

void ez_sprintf_destroy(ez_sprintf_t *ezsp)
{
	memset(ezsp, 0, sizeof(ez_sprintf_t));
}
