
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
** This is a generic event manager which maintains lists of registrants
** (function pointers & arguments), which will be called one by one
** for the specific object & events for which they are registered.
**
** The use of this module is specifically tailored to be used by the
** object manager although generic uses are not excluded if its
** limitations and idiosyncrasies are taken into account.
**
** Currently events are divided into two PRECISE categories:
**
**  - object events are events which are generated when an object
**    is created or deleted.  
**
**  - attribute events are events which are generated when a new
**    attribute id is added to an object or an existing attribute id
**    is deleted from an object or when a value changes in the attribute.
**
** Users can register to be notified of both of these types of events
** for a desired object TYPE.
**
** Note that users can register for either of these events based ONLY
** on the type of the object (not the instance).  Including filtering
** also based on instance numbers would have made the system too complex
** and extremely granular.  Having said that, when an event is reported,
** the event record WILL contain the instance of the object that the
** event applies to.
**
** There are two ways of registering for either type of events.
** One is to register in a way so that events are reported for
** ANY object type.  The other is such that events involving ONLY
** ONE specified type of object is of interest.
**
** So, a user can register/un-register to receive events for all the following 
** combinations:
**
**  - object events for ANY object type
**  - object events for SPECIFIC object types
**  - attribute events for ANY object type
**  - attribute events for SPECIFIC object types
**
** A user can register as many times, with as many object types and as many
** callbacks as possible.  There is no capacity restriction but there are
** consequences for duplicate registrations.  For example, if a user 
** registers for events for ALL objects, later user should NOT register
** for a specific object type for the same event, or vice versa.
** Otherwise the event manager will report the same event multiple times, 
** or as many times as the match occurs.  Event manager does NOT check
** redundant registrations.  It is up to the user to ensure that such
** logical errors are not made when registering for events.  Having said
** that, the event manager catches a duplicate registration when both the
** callback function AND the parameter is IDENTICAL to one which has been
** registered for the same type of object.  There is also a tool function
** which the user can use to find a duplicate registration.
**
*******************************************************************************
*******************************************************************************
*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "lock_object.h"
#include "ordered_list.h"
#include "index_object.h"
#include "object_types.h"

/*
 * These are *ALL* the possible events which can happen
 * These definitions are typically used with the event
 * manager & object manager code.
 */

#define OBJECT_CREATED                  (1 << 0)
#define OBJECT_DESTROYED                (1 << 1)
#define OBJECT_EVENTS \
    (OBJECT_CREATED | OBJECT_DESTROYED)

#define ATTRIBUTE_INSTANCE_ADDED        (1 << 5)
#define ATTRIBUTE_INSTANCE_DELETED      (1 << 6)
#define ATTRIBUTE_INSTANCE_EVENTS \
    (ATTRIBUTE_INSTANCE_ADDED | ATTRIBUTE_INSTANCE_DELETED)

#define ATTRIBUTE_VALUE_ADDED           (1 << 10)
#define ATTRIBUTE_VALUE_DELETED         (1 << 11)
#define ATTRIBUTE_VALUE_EVENTS \
    (ATTRIBUTE_VALUE_ADDED | ATTRIBUTE_VALUE_DELETED)

#define ATTRIBUTE_EVENTS \
    (ATTRIBUTE_INSTANCE_EVENTS | ATTRIBUTE_VALUE_EVENTS)

static inline int
is_an_object_event (int event)
{ return (event & OBJECT_EVENTS); }

static inline int
is_an_attribute_value_event (int event)
{ return (event & ATTRIBUTE_VALUE_EVENTS); }

static inline int
is_an_attribute_instance_event (int event)
{ return (event & ATTRIBUTE_INSTANCE_EVENTS); }

static inline int
is_an_attribute_event (int event)
{ return (event & ATTRIBUTE_EVENTS); }

/*
 * This structure is used to notify every possible event that could
 * possibly happen in the system.  It is passed around as a parameter
 * when entities are interested in being notified of events.
 * ALL the possible events fall into the events defined above.
 *
 * This is a variable size structure whose total length (in bytes) is
 * always specified in its FIRST field so that the entity being
 * notified will know how to read the entire record.
 *
 * The structure includes all the information needed to process
 * the event completely and it should NOT contain any pointers
 * in case events are notified across different processes.
 *
 * The 'event_type' field determines which part of the rest of the structure
 * elements are needed (or ignored).
 *
 * One possible explanation is needed in the case of an attribute event.
 * If this was an attribute event which involves a variable size
 * attribute value, the length will be specified in 'attribute_value_length'
 * and the byte stream would be available from '&attribute_value_data'
 * onwards.  This makes the structure of variable size, based on
 * the parsed parameters, with a complex attribute directly attached to the
 * end of the structure.  This is why the structure is a variable sized
 * structure.
 */
typedef struct event_record_s {

    /*
     * How many total bytes is this event record ?
     * This MUST be first field since during reads & writes, it needs
     * to be determined at the very beginning.  It will also be used
     * to copy the entire structure since its length is variable.
     */
    int total_length;

    /*
     * What is this event ?  Chosen from one of the
     * possibilities above.
     */
    int event_type;

    /* optional: which object manager this event/command applies to */
    int manager_id;

    /* object to which the event is directly relevant to */
    int object_type;
    int object_instance;

    /*
     * if the event involves another object, this is that OTHER object.
     * Not used if another object is not involved.
     *
     * One example of when it is used is during an object creation event.
     * In this situation, the 'object_type' & 'object_instance' will
     * represent the object actually created and 'related_object_type'
     * and 'related_object_instance' will represent its parent.
     *
     * An example of when it is NOT needed is during an object deletion
     * event.  In this case, no other object is involved and these
     * are not used (This is actually not STRICTLY true since when an
     * object is deleted, its relationship from its parent must be severed.
     * However, in this case, the system already knows about it and the parent
     * does not need to be specified).
     */
    int related_object_type;
    int related_object_instance;

    /* if the event involves an attribute, these are also used */
    int attribute_id;
    int attribute_value_length;
    long long int attribute_value_data;

    /* rest of data if it exists */
    unsigned char extra_data [0];

} event_record_t;

/*
 * This is the function which gets registered by users which handles
 * the event.  The event manager passes in the event which caused it
 * to be called first, followed by the transparent user pointer.
 */
typedef void (*event_handler_t)(event_record_t *evrp, void *extra_arg);

typedef struct event_manager_s {

    LOCK_VARIABLES;
    MEM_MON_VARIABLES;

    /*
     * while traversing lists and calling the user registered callbacks,
     * it has to be ensured that the callbacks can NOT cause changes and
     * corrupt the lists.
     */
    int should_not_be_modified;

    /*
     * list of registrants interested in object events (object creation
     * and deletion) for ANY type of object.
     */
    ordered_list_t object_event_registrants_for_all_objects;

    /*
     * list of registrants interested in attribute
     * events (attribute id add/delete, attribute value
     * add/delete) for ANY type of object.
     */
    ordered_list_t attribute_event_registrants_for_all_objects;

#ifdef CONSECUTIVE_OBJECT_TYPES_USED

    /*
     * list of registrants interested in object
     * events for ONE specific type of object.
     * index is the object_type.
     */
    ordered_list_t object_event_registrants_for_one_object [OBJECT_TYPE_SPAN];

    /*
     * list of registrants interested in attribute
     * events for ONE specific type of object.
     * index is the object_type.
     */
    ordered_list_t attribute_event_registrants_for_one_object [OBJECT_TYPE_SPAN];

#else
    
    index_obj_t object_event_registrants_for_one_object;
    index_obj_t attribute_event_registrants_for_one_object;

#endif

} event_manager_t;

extern int
event_manager_init (event_manager_t *emp,
    int make_it_thread_safe,
    mem_monitor_t *parent_mem_monitor);

/*
 * Determines if the EXACT COMBINATION of user callback function AND
 * the opaque parameter is already registered for the specified
 * object & event type.  Returns 1 (true) if so, else 0 (false).
 */
extern int
already_registered (event_manager_t *emp,
    int event_type, int object_type,
    event_handler_t evhfptr, void *extra_arg);

/*
 * This function registers the caller to be notified of object events
 * (object creation & deletion) for either a specific object TYPE (not
 * also an instance, we cannot do that much granularity) or all object
 * types. If 'object_type' is ALL_OBJECT_TYPES, then object events for
 * every object type will be reported.  If it is a specific object, then
 * object events only for that object type will be reported.
 *
 * When a caller registers for events for an object, he supplies
 * an event callback function and an opaque parameter.  When the
 * event occurs, the callback will be called with the FIRST parameter
 * as the event_record_t parameter explaining the details of the
 * event and the SECOND parameter as the user specified opaque 
 * parameter which was specified at the time of registration.
 *
 * Note that since this is a registration only for object events,
 * the event record passed into the callback function will be 
 * GUARANTEED to be of only object creation & deletion events.
 */
extern int
register_for_object_events (event_manager_t *emp,
    int object_type, 
    event_handler_t evhfptr, void *extra_arg);

/*
 * unregisters from object events for the specified object type.
 * Basically, reverse of the above.
 */
extern void
un_register_from_object_events (event_manager_t *emp,
    int object_type,
    event_handler_t evhfptr, void *extra_arg);

/*
 * Same concept as above but this time registration is only for attribute
 * events, which are attribute id add/delete and attribute value add/delete.
 *
 * Similar to above, the event record passed into this callback function
 * will be GUARANTEED to be only of attribute events.
 */
extern int
register_for_attribute_events (event_manager_t *emp,
    int object_type, 
    event_handler_t evhfptr, void *extra_arg);

/*
 * reverse of the above
 */
extern void
un_register_from_attribute_events (event_manager_t *emp,
    int object_type,
    event_handler_t evhfptr, void *extra_arg);

/*
 * The user calls this when he wants to report the occurence of an event.
 * Based on the event type, object type in the event record, all the
 * registered functions will be invoked one by one.
 */
extern void
announce_event (event_manager_t *emp, event_record_t *erp);

extern void
event_manager_destroy (event_manager_t *emp);

#ifdef __cplusplus
} // extern C
#endif 

#endif // __EVENT_MANAGER_H__


