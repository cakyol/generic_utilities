
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

#include "line_counters.h"

#define PUBLIC

PUBLIC void
init_line_counter_block(line_counter_block_t *lcbp)
{
	memset(lcbp, 0, sizeof(*lcbp));
}

/*
 * compares the keys of two line counter pointers as if they were simply
 * a string of bytes and returns 0, <0 and >0 for equal, lt & gt.
 */
static inline int
compare_line_counters(line_counter_t *e1, line_counter_t *e2)
{
	int result = e1->line_number - e2->line_number;
	if (result) return result;

	/*
	 * This makes a HUGE performance improvement.  Good compilers
	 * store static strings in a string manager and hence store
	 * the same strings once.  This short circuits the strcmp below.
	 */
	if (e1->function_name == e2->function_name) return 0;

	return strcmp(e1->function_name, e2->function_name);
}

/*
 * The hash function uses the line number for a good spread.  The
 * line numbers are extremely unlikely to be same so this is expected
 * to give a very good spread in the hash table.
 */
static inline int
line_counter_hash(line_counter_t *lcp)
{
	return
		lcp->line_number % MAX_LINE_COUNTERS;
}

/*
 * Given the function_name & line number, this function finds the corresponding
 * counter in the line_counter_block_t.  If it cannot be found, this must be
 * a new call, so it creates the entry.  It then increments the value.
 * If a new entry in the array is consumed, it advances the entry count (n).
 */
PUBLIC int
increment_line_counter(line_counter_block_t *lcbp,
	const int line_number, const char *function_name)
{
	line_counter_t *newone, *found;
	int index;
	int bsize = 0;

	if (0 == lcbp) return EINVAL;

	/*
	 * next potential entry if a new one.  It will be
	 * discarded if an existing one is found
	 */
	newone = &lcbp->lc_array[lcbp->n];
	newone->line_number = line_number;
	newone->function_name = function_name;

	index = line_counter_hash(newone);
	found = lcbp->lc_hash[index];

	/* search thru buckets */
	while (found) {
		if (compare_line_counters(found, newone) == 0) break;
		bsize++;
		found = found->next;
	}

	if (found) {

		/* update the longest bucket size & index if needed */
		if (bsize > lcbp->longest_bucket_size) {
			lcbp->longest_bucket_size = bsize;
			lcbp->longest_bucket_size_index = index;
		}

		found->value++;

		return 0;
	}

	/* not found, attempt to insert if space exists in the hash table */
	if (lcbp->n >= MAX_LINE_COUNTERS) {
		lcbp->insertion_failed++;
		return ENOSPC;
	}

	/* insert to head of list, easiest & fastest way */
	newone->next = lcbp->lc_hash[index];
	lcbp->lc_hash[index] = newone;

	newone->value = 1;
	lcbp->n++;

	return 0;
}
