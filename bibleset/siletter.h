/* siletter */

/* perform some kind of processsing */


/* revision history:

	= 2009-04-01, David A­D­ Morano
        This subroutine was written as an enhancement for adding back-matter
        (end pages) to the output document.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SILETTER_INCLUDE
#define	SILETTER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	SILETTER		struct siletter


struct siletter {
	const char	*lp ;
	int		ll ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	siletter(SILETTER *,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SILETTER_INCLUDE */


