
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

#ifndef __LINE_COUNTERS_H__
#define __LINE_COUNTERS_H__

/*
 * This structure represents how many times a specific line in the
 * indicated function has been executed during the running of the code.
 *
 * The entry 'insertion_failed' is used to record how many times
 * an entry could not be inserted into the hash table becoz the 'lc_array'
 * table size was not big enuf.  If this value is > 0, then the value defined
 * by MAX_LINE_COUNTERS must be increased to a bigger prime number.
 */
typedef struct line_counter_s line_counter_t;
struct line_counter_s {

	line_counter_t *next;
	const char *function_name;
	int line_number;
	long long int value;
};

/* must be prime for better hash distribution */
#define MAX_LINE_COUNTERS		1021

typedef struct line_counter_block_s {

	/* how many line counters in the block are in use */
	int n;

	/* actual counters (+1 is used as the scratchpad) */
	line_counter_t lc_array [MAX_LINE_COUNTERS + 1];

	/* hash buckets */
	line_counter_t *lc_hash [MAX_LINE_COUNTERS];

	/* used for diagnostics/statistics of the hash table */
	long long int insertion_failed;
	int longest_bucket_size;
	int longest_bucket_size_index;

} line_counter_block_t;

/*
 * This is a function pointer declaration which is needed in
 * situations where a function is needed in a loaded module
 * which does not yet exist until another module is loaded.
 * This became an issue in the pktfree tracing feature.  This
 * pointer approach eliminates this problem.  The pointer is
 * used in this module (but not while it is NULL) and when the
 * other module gets loaded, the pointer is set and the pointer
 * can now be used with the proper function.
 */
typedef line_counter_block_t* (*get_line_counter_block_fptr)(void *p);

/* initializes a line counter block, all entries are zeroed out */
extern void init_line_counter_block(line_counter_block_t *lcbp);

/* records an execution into the given block at the indicated function & line */
extern int increment_line_counter(line_counter_block_t *lcbp,
	const int line_number, const char *function_name);

#endif /* __LINE_COUNTERS_H__ */
