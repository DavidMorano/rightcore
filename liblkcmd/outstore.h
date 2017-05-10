/* outstore */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OUTSTORE_INCLUDE
#define	OUTSTORE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<localmisc.h>


#define	OUTSTORE	struct outstore
#define	OUTSTORE_SLEN	128


struct outstore {
	int		dlen ;
	int		len ;
	char		*dbuf ;	/* dynamically sized (allocated) */
	char		sbuf[OUTSTORE_SLEN+1] ; /* "static" or "stack" */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	outstore_start(OUTSTORE *) ;
extern int	outstore_strw(OUTSTORE *,cchar *,int) ;
extern int	outstore_get(OUTSTORE *,cchar **) ;
extern int	outstore_clear(OUTSTORE *) ;
extern int	outstore_finish(OUTSTORE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* OUTSTORE_INCLUDE */


