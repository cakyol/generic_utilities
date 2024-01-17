
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
** Singly linked list used as a lifo.
**
** To keep things simple and extremely fast, insertions are ALWAYS
** performed to the head of the list and thats it (hence the name lifo).
**
** In this implementation, when a node is to be deleted, to speed up
** the execution, the NEXT node is copied over to this one and the NEXT
** node is deleted and voila, it is done.  No search to find the previous
** node is necessary.  For this scheme to be successful, an always
** present 'end node' must be defined, which denotes the end of lifo,
** rather than simply using a NULL like most lists do.  This 'end of list'
** is implemented by having BOTH the 'next' and 'data' pointers of
** a node as NULL.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <errno.h>
#include <assert.h>

#include "common.h"
#include "mem_monitor_object.h"
#include "lock_object.h"
#include "debug_framework.h"

typedef struct lifo_node_s lifo_node_t;
typedef struct lifo_s lifo_t;

struct lifo_node_s {
    lifo_node_t *next;
    void *data;
};

struct lifo_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;
    STATISTICS_VARIABLES;

#ifdef EXTRA_CONSISTENCY_CHECKS
    lifo_t *my_lifo;
#endif /* EXTRA_CONSISTENCY_CHECKS */

    /* size limit of lifo.  If 0, no limit */
    unsigned int n_max;

    lifo_node_t *head;
    unsigned int n;

};

#define END_NODE(n)     ((NULL == n->next) && (NULL == n->data))

extern int
lifo_init (lifo_t *lifo,
    bool make_it_thread_safe,
    bool enable_statistics,
    unsigned int n_max,
    mem_monitor_t *parent_mem_monitor);

extern int
lifo_add_data (lifo_t *lifo, void *data, lifo_node_t **node_returned);

extern int
lifo_search_data (lifo_t *lifo, void *data, lifo_node_t **node_returned);

extern int
lifo_remove_node (lifo_t *lifo, lifo_node_t *node);

int lifo_remove_data (lifo_t *lifo, void *data);

extern void
lifo_destroy (lifo_t *lifo);

#ifdef __cplusplus
} // extern C
#endif


