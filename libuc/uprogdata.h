/* uprogdata */

/* UNIX® program-data cache */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UPROGDATA_INCLUDE
#define	UPROGDATA_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>


/* registery */

#define	UPROGDATA_DNETLOAD	0	/* netload */
#define	UPROGDATA_DSYSTAT	1	/* sys-stat */


#if	(! defined(UPROGDATA_MASTER)) || (UPROGDATA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uprogdata_init() ;
extern int uprogdata_set(int,cchar *,int,int) ;
extern int uprogdata_get(int,char *,int) ;
extern void uprogdata_fini() ;

#ifdef	__cplusplus
}
#endif

#endif /* UPROGDATA_MASTER */

#endif /* UPROGDATA_INCLUDE */


