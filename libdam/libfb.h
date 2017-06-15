/* libfb */

/* file-binary operations (suppliment to STDIO) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LIBFB_INCLUDE
#define	LIBFB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	<localmisc.h>


#if	(! defined(LIBFB_MASTER)) || (LIBFB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int fbprint(FILE *,const char *,int) ;
extern int fbprintf(FILE *,const char *,...) ;
extern int fbreadline(FILE *,char *,int) ;
extern int fbread(FILE *,void *,int) ;
extern int fbwrite(FILE *,const void *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* LIBFB_MASTER */

#endif /* LIBFB_INCLUDE */


