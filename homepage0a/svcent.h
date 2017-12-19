/* svcent INCLUDE */

/* subroutines for simple SVCFILE_ENT object management */


/* revision history:

	= 2017-10-13, David A­D­ Morano
	This was split out of the HOMEPAGE program (where it was originally
	local).

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

#ifndef	SVCENT_INCLUDE
#define	SVCENT_INCLUDE	1


#include	<envstandards.h>
#include	<svcfile.h>
#include	<localmisc.h>


#define	SVCENT	SVCFILE_ENT


#ifdef	__cplusplus
extern "C" {
#endif

extern int	svcent_islib(SVCENT *,cchar **) ;
extern int	svcent_getval(SVCENT *,cchar *,cchar **) ;
extern int	svcent_getdeval(SVCENT *,cchar *,cchar **) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCENT_INCLUDE */


