
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
**
** File io from inside the linux kernel.
**
******************************************************************************/

#ifndef __LINUX_FILEIO_H__
#define __LINUX_FILEIO_H__

#include <linux/fcntl.h>

extern int linux_kernel_file_write (char *filename,
    int flags, void *buf, int size);

extern void linux_kernel_file_rewind (char *filename);

static inline int
linux_kernel_file_append (char *filename, void *buf, int size)
{
	return
		linux_file_write(filename,
			(O_WRONLY | O_CREAT | O_APPEND | O_SYNC),
			buf, size);
}

#endif /* __LINUX_FILEIO_H__ */

