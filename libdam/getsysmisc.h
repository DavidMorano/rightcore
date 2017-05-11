/* getsysmisc */
/* get SYSMISC information from the kernel */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	GETSYSMISC_INCLUDE
#define	GETSYSMISC_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	GETSYSMISC	struct getsysmisc


struct getsysmisc {
	uint	btime ;
	uint	ncpu ;
	uint	nproc ;
	uint	la[3] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int getsysmisc(GETSYSMISC *,time_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETSYSMISC_INCLUDE */


