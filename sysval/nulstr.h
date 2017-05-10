/* nulstr */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NULSTR_INCLUDE
#define	NULSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


#define	NULSTR		struct nulstr
#define	NULSTR_SHORTLEN	128


struct nulstr {
	const char	*as ;	/* allocated string */
	char		buf[NULSTR_SHORTLEN+1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	nulstr_start(NULSTR *,const char *,int,const char **) ;
extern int	nulstr_finish(NULSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NULSTR_INCLUDE */


