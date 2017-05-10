/* uc_mallocstrw */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_STRNCPY	1		/* use |strwcpy(3dam)| */


/* revision history:

	= 1998-03-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is similar to 'uc_malloc(3uc)' except that it takes a
        string argument and copies it into the newly allocated space.

	Synopsis:

	int uc_mallocstrw(sp,sl,rpp)
	const char	sp[] ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp		string pointer
	sl		string length
	rpp		resulting pointer

	Returns:

	>=0		nominally -> (strlen(string) + 1)
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int uc_mallocstrw(cchar *sp,int sl,cchar **rpp)
{
	int		rs ;
	int		size ;
	char		*bp ;

	if (rpp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (sl < 0) sl = strlen(sp) ;

	size = (sl + 1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    *rpp = bp ;
#if	CF_STRNCPY
	    strncpy(bp,sp,sl) ;
	    bp[sl] = '\0' ;
#else
	    while (sl-- && *sp) *bp++ = *sp++ ;
	    *bp = '\0' ;
#endif /* CF_STRNCPY */
	}

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_mallocstrw) */


