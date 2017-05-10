/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<vecelem.h>


#ifndef	TYPEDEF_UTIMET
typedef unsigned long	utime_t	;
#define	TYPEDEF_UTIMET
#endif



#define	DATE1970	((24 * 10) * 3600)

#define BUFSIZE		((2*MAXPATHLEN) + 4)
#define	LINELEN		200


#if	defined(BSD)
#define	MAP_FAILED	((void *) (-1))
#endif




struct gflags {
	uint	verbose : 1 ;
	uint	sysv_rt : 1 ;
	uint	sysv_ct : 1 ;
	uint	daemon : 1 ;
	uint	tmpfile : 1 ;
	uint	quiet : 1 ;
	uint	alternate : 1 ;
} ;

struct global {
	bfile		*efp ;
	struct gflags	f ;
	logfile	lh ;
	int		debuglevel ;
	char		*progname ;
	char		*programroot ;		/* program root */
	char		*tmpdir ;
	char		*mskfname ;
} ;


#endif /* DEFS_INCLUDE */


