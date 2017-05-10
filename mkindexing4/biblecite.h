/* biblecite */

/* bible-citation */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	BIBLECITE_INCLUDE
#define	BIBLECITE_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	BIBLECITE	struct biblecite


struct biblecite {
	uint	b, c, v ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	isbiblecite(BIBLECITE *,const char *,int,int *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLECITE_INCLUDE */


