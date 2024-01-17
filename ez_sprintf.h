
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

#ifndef __EZ_SPRINTF_H__
#define __EZ_SPRINTF_H__

/*
 * Makes sprintf easy to use.  User can simply add formatted
 * statements and this object will keep a count of everything.
 */
typedef struct ez_sprintf_s {

	char *buffer;			/* the write buffer itself */
	int max_size;			/* max size of the buffer (constant) */
	int remaining_size;		/* how many more bytes can be written */
	int write_idx;			/* current point in buffer to start writing into */
	int indent_size;		/* indent increment */
	int current_indent;

} ez_sprintf_t;

/*
 * initializes an ez sprintf object to a buffer which is externally 
 * provided with the given size.
 */
extern void ez_sprintf_init_with_external_buffer(ez_sprintf_t *ezsp,
	char *externally_supplied_buffer, int external_buffer_size,
	int indent_size);

/* appends printf formatted output and updates all its control variables */
extern int ez_sprintf_append(ez_sprintf_t *ezsp, char *fmt, ...);

/*
 * increments the current indent by indent_size_override if not 0,
 * else by the default indent size specified at init time.
 */
static inline void
ez_sprintf_incr_indent(ez_sprintf_t *ezsp, int indent_size_override)
{
	ezsp->current_indent +=
		indent_size_override ?
			indent_size_override : ezsp->indent_size;
}

/* decrements the current indent with the same logic as in ez_sprintf_incr_indent */
static inline void
ez_sprintf_decr_indent(ez_sprintf_t *ezsp, int indent_size_override)
{
	ezsp->current_indent -=
		indent_size_override ?
			indent_size_override : ezsp->indent_size;
	if (ezsp->current_indent < 0) ezsp->current_indent = 0;
}

/* outputs the indent */
extern int
ez_sprintf_indent(ez_sprintf_t *ezsp);

/* resets it (empties) an ez sprintf object */
extern void ez_sprintf_reset(ez_sprintf_t *ezsp);

/* destroys the object, frees its buffer */
extern void ez_sprintf_destroy(ez_sprintf_t *ezsp);

/*
 * usage example:
 *
	ez_sprintf_t ezs;
	char buffer [5000];

	ez_sprintf_init_with_external_buffer(&ezs, buffer, 5000, 4);
	ez_sprintf_append(&ezs, "first line %d\n", 1);
	ez_sprintf_indent&ezs); ez_sprintf_incr_indent(&ezs);
	ez_sprintf_append(&ezs, "second line %d\n", 2);
	ez_sprintf_indent&ezs); ez_sprintf_incr_indent(&ezs);
	ez_sprintf_append(&ezs, "third line %d\n", 3);
	ez_sprintf_append(&ezs, "fourth line %d\n", 4);
	ez_sprintf_append(&ezs, "and the last line %d %d %d %d%s", 13, 14, 15, 16, "\n");
	printf(ezs.buffer);
 */

#endif /* __EZ_SPRINTF_H__ */
