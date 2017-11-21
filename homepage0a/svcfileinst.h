/* svcfileinst INCLUDE */

/* subroutines for simple SVCFILE_ENT object management */


/* revision history:

	= 2017-10-13, David A­D­ Morano
	This was split out of the HOMEPAGE program (where it was originally
	local).

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

#ifndef	SVCFILEINST_INCLUDE
#define	SVCFILEINST_INCLUDE	1


#include	<envstandards.h>
#include	<svcfile.h>
#include	<localmisc.h>


#define	SVCFILEINST	SVCFILE_ENT


#ifdef	__cplusplus
extern "C" {
#endif

extern int	svcfileinst_val(SVCFILE_ENT *,cchar *,cchar **) ;
extern int	svcfileinst_deval(SVCFILE_ENT *,cchar *,cchar **) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCFILEINST_INCLUDE */


