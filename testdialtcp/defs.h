/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


#define	NPARG		2
#define	BUFLEN		MAXPATHLEN

#define	TIME_SLEEP	7
#define	MAIL_TICS	3
#define	FULL_TICS	100

#define	N		NULL


#endif /* DEFS_INCLUDE */


