
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

#ifndef __OBJECT_TYPES_H__
#define __OBJECT_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * All objects are uniquely identified by a tuple of object type
 * and an object instance number. Zeros are reserved for the
 * root of the system and should NEVER be used for any user defined
 * objects.  User objects should always have type & instance values 
 * of greater than 0.
 *
 * Keeping object types starting from 1 and making them consecutive
 * helps in saving memory when using dynamic arrays.  Unless absolutely
 * necessary this is a very good rule to follow.
 *
 * Note that these values also mean 'all' types and instances for 
 * certain contexts.
 */
#define ROOT_OBJECT_TYPE                0
#define ALL_OBJECT_TYPES                ROOT_OBJECT_TYPE
#define ROOT_OBJECT_INSTANCE            0
#define ALL_OBJECT_INSTANCES            ROOT_OBJECT_INSTANCE

/*
 * Define all YOUR object TYPES here, hopefully as consecutively as possible,
 * starting from 1 and upwards.  Since there are arrays of these defined
 * in the code, it keeps the array sizes small and manageable.
 * Always define MIN_OBJECT_TYPE & MAX_OBJECT_TYPE since the array bounds
 * are determined by these values.  DO NOT USE 0 as object type & instance.
 * It is special.  Always start from 1.
 *
 * If your object types are consecutive and not too many, define 
 * CONSECUTIVE_OBJECT_TYPES_USED so that event manager gets a hint 
 * to use arrays and work MUCH faster.  If however, your object types
 * are all over the place with very large numbers and not consecutive,
 * then do NOT define CONSECUTIVE_OBJECT_TYPES_USED.
 */
#define MIN_OBJECT_TYPE                 1
    /*
     * define all YOUR OWN objects here within these limits.
     * An array will be defined in this range so try and keep
     * the objects as sequential as possible starting from 1.
     * This way, the array memory used will be optimal.
     */
#define MAX_OBJECT_TYPE                 4096 /* change this as you wish */
#define OBJECT_TYPE_SPAN                (MAX_OBJECT_TYPE + 1)

//#define CONSECUTIVE_OBJECT_TYPES_USED

static inline int
object_type_within_limits (int object_type)
{
    return
        (object_type >= MIN_OBJECT_TYPE) &&
        (object_type <= MAX_OBJECT_TYPE);
}

#ifdef __cplusplus
} // extern C
#endif 

#endif // __OBJECT_TYPES_H__


