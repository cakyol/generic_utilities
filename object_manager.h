
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
** @@@@@ GENERIC OBJECT MANAGER
**
**      This is a hierarchical object manager.  It is very fast
**      to create, delete and search an object and/or its attributes.
**
**      An object is uniquely identified by two integers, its type and
**      its instance.
**      
**      An object (as defined by its uniqueness) can only appear ONCE
**      in the manager.
**
**      An object MUST have a parent (except the root object) and may or
**      may not have any children.
**
**      An object can have 0 or any number of attributes and these can be
**      added and/or deleted dynamically during an object's lifetime.
**      Each attribute is identified by an integer.  Attribute ids MUST
**      be unique per object but do not have to be unique for the
**      entire manager.  Object attributes and their values can be added
**      and deleted any time from an object.
**
**      Each attribute can have a value which is in the form of a length
**      and a sequence of bytes of that length.  An attribute can exist
**      without a value having been assigned to it.  This would be
**      represented by a length of 0.
**
**      An object can also add and delete children at any time.
**
**      If an object is destroyed, all its attributes and children will
**      also be destroyed in its entirety.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __OBJECT_MANAGER_H__
#define __OBJECT_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "debug_framework.h"
#include "mem_monitor_object.h"
#include "lock_object.h"
#include "avl_tree_object.h"
#include "index_object.h"
#include "lifo.h"
#include "assert.h"

#define TYPICAL_NAME_SIZE                       (64)

typedef struct attribute_s attribute_t;
typedef struct object_identifier_s object_identifier_t;
typedef struct object_representation_s object_representation_t;
typedef struct object_s object_t;
typedef struct object_manager_s object_manager_t;

/******************************************************************************
 *
 * attribute related structures
 *
 */

/*
 * This is an attribute instance object which also embodies the value
 * of the attribute in the same structure.
 *
 * An example of an attribute instance can be 'port speed'.  It may
 * have a simple attribute value of 100M to represent 100 Mbits.
 *
 * Note that when the attribute value is 'attached' to the bottom
 * of the structure.  Depending on the length, enough number of bytes
 * will be allocated when the attribute is created.
 *
 * If a null terminated string is to be stored as an attribute value,
 * the terminating 0 MUST be included in the length since the
 * system does not attach any meaning to the values.  So always store
 * the value with 'strlen() + 1", to include the terminating 0 of the
 * string.
 */
struct attribute_s {
    object_t *object;               /* object to which this attribute belongs */
    int attribute_id;               /* which attribute is it */
    int attribute_value_length;     /* the length of its value in bytes */
    byte attribute_value_data [0];  /* the attribute value itself in bytes */
};

/******************************************************************************
 *
 * object related structures
 *
 */

/*
 * User facing APIs mostly deal with this representation of an object
 * since pointers are mostly hidden from the user and the only way
 * to address an object is thru this.  This reduces the chance of a
 * user program corrupting pointers or accessing them incorrectly.
 */
struct object_identifier_s {
    int object_type;
    int object_instance;
};

/*
 * Internal APIs mostly use pointers since they are more protected.
 * and pointers are safe to use.  So, this one type can be used both 
 * for user facing APIs as well as internal uses.
 */
struct object_representation_s {

    /* set if the representation is a pointer */
    tinybool is_pointer;

    /* which address type to use */
    union {
        object_identifier_t object_id;
        object_t *object_ptr;
    } u;

};

struct object_s {

    /* which object manager this object belongs to */
    object_manager_t *omp;

    /*
     * Unique identification of this object.  This combination is
     * always unique per object manager and distinctly identifies an object.
     * This combination is always the unique 'key'.
     */ 
    int object_type;
    int object_instance;

    /*
     * Parent of this object.  If object is root, it has no parent.
     */ 
    object_representation_t parent;

    /* All the children of this object */
    lifo_t children;

    /*
     * This is how I am represented in my parents' lifo list above me.
     * This makes deletion of an object very fast, when it is required
     * that my own existence is erased from my parents' list.  This is
     * the handle which represents me in the parents' list object 'children'.
     * Note that this will be NULL for the root object.
     */
    lifo_node_t *child_handle;

    /*
     * All the attributes of this objectllows very fast retreival of
     * attributes altho they would be slower to insert & delete but
     * it is not expected to add/delete attributes to an object very
     * often.  The VALUE of an attribute may be changed but its existence
     * will not be so dynamic.
     */
    index_obj_t attributes;

};

/******************************************************************************
 *
 * object manager related structures
 *
 */

struct object_manager_s {

    MEM_MON_VARIABLES;
    LOCK_VARIABLES;

    /* unique integer for this manager */
    int manager_id;

    /* if a traversal is taking place, the object manager cannot be modified */
    boolean busy;

    /* the actual object tree This ALWAYS has (0, 0) type, instnce */
    object_t *root;

    /*
     * This is the OBJECT DIRECT lookup table.  Note that this
     * is NOT the parent/child tree.  It is used ONLY for fast
     * direct lookup of an object.
     *
     * Objects are always uniquely indexed by 'object_type' & 
     * 'object_instance'.  There can be only ONE object of
     * this unique combination in every manager.  They are
     * kept in this avl tree for fast access.
     */
    avl_tree_t om_objects;

}; 

/************* User functions ************************************************/

/* debug infrastructure */
extern debug_module_block_t om_debug;

/*
 * initialize object manager
 */
extern int
om_init (object_manager_t *omp,
    boolean make_it_thread_safe,
    int manager_id,
    mem_monitor_t *parent_mem_monitor);

/*
 * creates an object with given id & type under the
 * specified parent.  Success returns 0.  If it already
 * exists, this is a no-op, unless the required parents
 * do not match the existing parents.  In that case,
 * the object creation is refused.
 */
extern int
om_object_create (object_manager_t *omp,
    int parent_object_type, int parent_object_instance,
    int object_type, int object_instance);

/*
 * returns true if the specified object exists in the object manager.
 */
extern bool
om_object_exists (object_manager_t *omp,
    int object_type, int object_instance);

static inline int
om_object_count (object_manager_t *omp)
{ return omp->om_objects.n; }

/*
 * add (modify if it already exists) an attribute (id) to an object.
 *
 * 'attribute_value_length' represents the attribute value length in
 * bytes.  'attribute_value' is the data itself in contiguous bytes.
 *
 * If the specified length is 0, then the attribute value will be deleted.
 * The attribute will still exist but it will not have a value.  It can
 * be used to 'clear' an attribute.
 *
 * If the specified length is > 0, then the attribute value will be set
 * to the new specified value, even if the attribute already had a value
 * previously.
 */
extern int
om_attribute_add (object_manager_t *omp,
    int object_type, int object_instance,
    int attribute_id,
    int attribute_value_length, byte *attribute_value);

/*
 * This will return true or false depending on whether
 * such an attribute exists in the object.
 */
extern bool
om_attribute_exists (object_manager_t *omp,
    int object_type, int object_instance,
    int attribute_id);

/*
 * This will return the attribute value requested.  It will return
 * the value in 'returned_value' and the length in 'returned_length'.
 * Caller MUST ensure that the buffer specified (by returned_value)
 * is large enough to accommodate the value.  This will be done by
 * the caller passing 'max_length' as the buffer length.  If 'max_length'
 * is less than the size necessary to fully copy the entire value of
 * the attribute, then the operation will fail and an error will be
 * returned from the function.  Otherwise a successful copy will
 * be done and a 0 will be returned as the function value.
 */
extern int
om_attribute_get (object_manager_t *omp,
    int object_type, int object_instance,
    int attribute_id,
    int *returned_length, int max_length, byte *returned_value);

/*
 * This will actually completely remove the specified attribute from
 * the specified object.  If the attribute was not present anyway, it
 * will be a no op.
 */
extern int
om_attribute_remove (object_manager_t *omp,
    int object_type, int object_instance,
    int attribute_id);

/*
 * get the parent type & instance of an object and
 * place them in the addresses respectively.
 * If not found, error is returned.  Success
 * returns 0.
 */
extern int
om_parent_get (object_manager_t *omp,
    int object_type, int object_instance,
    int *parent_object_type, int *parent_object_instance);

/*
 * Destroys an object and all its children, including attributes,
 * values, everything.  Note that simply taking out the object from
 * the name search tree 'om_objects" and removing it from the parents'
 * children list is very fast and enough to achieve this.
 * The actual sub-tree will be completely isolated and will simply
 * be traversed to release all its sub objects/attributes in a separate
 * low priority thread in the back ground.
 */
extern int
om_object_remove (object_manager_t *omp,
    int object_type, int object_instance);

/*
 * traverses the object AND all its children applying the
 * function 'tfn' to all of them.  The parameters passed 
 * to the 'tfn' function will be:
 *
 *      param0: object manager pointer
 *      param1: the child pointer being traversed
 *      param2: p0
 *      param3: p1
 *      param4: p2
 *      param5: p3
 *      param6: p4
 *
 * The return value is the first error encountered by 'tfn'.
 * 0 means no error was seen.
 */
extern int
om_traverse (object_manager_t *omp,
    int object_type, int object_instance,
    traverse_function_pointer tfn,
    void *p0, void *p1, void *p2, void *p3, void *p4);

/*
 * destroys the entire object manager.  It cannot be used again
 * until re-initialised.
 */
extern void
om_destroy (object_manager_t *omp);

/******************************************************************************
 *
 *  Object manager reading & writing into a file.
 *
 *  The object manager in a file conforms to the below format:
 *
 *  - The filename is formed by concatanating "om_" & id number,
 *    such as "om_29".
 *
 *  - One or more object records where each one is an object,
 *    its parent and all its attributes and attribute values.
 *
 *  OPTIONAL
 *  - Last entry is a checksum.  This stops the object manager file from being
 *    hand modified.
 *
 *  Everything is in readable text.  All numbers are in decimal.
 *  The format is one or more objects grouped like below and repeated
 *  as many times as there are objects, where each object is separated
 *  by blank lines to improve readability.  The line breaks are delimiters.
 *
 *  OBJ %d %d %d %d
 *      (parent type, instance, object type, instance)
 *  AID %d
 *      (attribute id)
 *  AV %d %d %d %d %d %d ....
 *      (attribute value, first int is the length & the rest
 *       are all the data bytes)
 *
 *  Parsing of an object is very context oriented.  When a 'OBJ' line
 *  is parsed, it defines the object context to which all consecutive
 *  lines apply, until another 'OBJ' line is reached where the context
 *  changes and so on till the end of the file.
 *  When an 'AID' is parsed it defines an new attribute id for the current
 *  object in context.
 *  When a 'AV' is encountered the attribute value is defined
 *  for the current attribute id in context.
 */

/*
 * Writes out the object manager to a file.
 */
extern int
om_write (object_manager_t *omp);

/*
 * reads a object manager from a file
 */
extern int
om_read (int manager_id, object_manager_t *omp);

extern void
om_destroy (object_manager_t *omp);

#ifdef __cplusplus
} /* extern C */
#endif 

#endif /* __OBJECT_MANAGER_H__ */


