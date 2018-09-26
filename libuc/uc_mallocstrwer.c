/* uc_mallocstrw */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_STRLEN	1		/* return (strlen(s) + 1) */


/* revision history:

	= 1998-03-15, David A­D­ Morano
	This subroutine was originally written.

	= 2005-07-16, David A.D. Morano
	This was enhanced to reduce the size of the allocation in those cases
	when the string passed in is actually much smaller than its declared
	length. So far, this version of this subroutine is not being used (as
	far as I know), so it is sort of just a proof of concept at the moment.
	
*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is similar to 'uc_malloc(3uc)' except that it takes a
        string argument and copies it into the newly allocated space.

	Synopsis:

	int uc_mallocstrw(s,slen,rpp)
	const char	s[] ;
	int		slen ;
	const char	**rpp ;

	Arguments:

	s		string pointer
	slen		string length
	rpp		resulting pointer

	Returns:

	>=0		nominally -> (strlen(string) + 1)
	<0		error

	Notes:
	
	+ This is a version that reduced the actual memory allocation if the
	actual length of the passed string is somewhat substantially smaller
	than its declared length.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>


/* local defines */

#define	MALLOCSTRW_OVERLEN	10	/* over-length amount before acting */


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

#if	CF_STRLEN
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    int		bl = (strwcpy(bp,sp,sl) - bp) ;
	    *rpp = bp ;
	    if ((sl-bl) > MALLOCSTRW_OVERLEN) {
		char	*nbp ;
		size = (bl+1) ;
		if ((rs = uc_malloc(size,&nbp)) >= 0) {
	    	    strncpy(nbp,bp,bl) ;
	    	    nbp[bl] = '\0' ;
	    	    *rpp = nbp ;
		} /* end if (uc_malloc) */
		uc_free(bp) ;
	    } /* end if (handle over-length) */
	} /* end if (m-a-f) */
#else /* CF_STRLEN */
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    strncpy(bp,sp,sl) ;
	    bp[sl] = '\0' ;
	    *rpp = bp ;
	}
#endif /* CF_STRLEN */

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_mallocstrw) */

