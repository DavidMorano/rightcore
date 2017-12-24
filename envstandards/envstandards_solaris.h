/* envstandards (Solaris) */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


/* these are user-settable compile-time switches */

#ifndef	ENVSTANDARDS_INCLUDE
#define	ENVSTANDARDS_INCLUDE	1


#ifndef	OSNAME
#define	OSNAME				SunOS
#endif

#ifndef	OSNAME_SunOS
#define	OSNAME_SunOS			8
#endif

#ifndef	OSNUM
#define	OSNUM				8
#endif

#ifndef	OSTYPE_SYSV
#define	OSTYPE_SYSV			1
#endif

/* Solaris® thing needed to couteract certain effects of POSIX® and X/OPEN® */
#ifndef	__EXTENSIONS__
#define	__EXTENSIONS__			1 
#endif

#ifndef	_REENTRANT
#define	_REENTRANT			1
#endif

#ifndef	_POSIX_C_SOURCE
#define	_POSIX_C_SOURCE			199506L 
#endif

#ifndef	_XOPEN_SOURCE
#define	_XOPEN_SOURCE			500
#endif

#ifndef	_POSIX_PTHREAD_SEMANTICS
#define	_POSIX_PTHREAD_SEMANTICS	1 
#endif

#ifndef	_POSIX_PER_PROCESS_TIMER_SOURCE
#define	_POSIX_PER_PROCESS_TIMER_SOURCE	1
#endif

#ifndef	_LARGEFILE_SOURCE /* 64-bit sized files */
#define	_LARGEFILE_SOURCE		1
#endif

#ifndef	_FILE_OFFSET_BITS /* 64-bit sized file-offsets ('offset_t' ?) */
#define	_FILE_OFFSET_BITS		64
#endif


#include	<syshas.h>


#endif /* ENVSTANDARDS_INCLUDE */


