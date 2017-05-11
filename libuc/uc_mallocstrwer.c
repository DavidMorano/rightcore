/* uc_mallocstrw */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_STRLEN	1		/* return (strlen(s) + 1) */


/* revision history:

	= 1998-03-15, David A­D­ Morano
	This subroutine was originally written.

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
	int		cl ;
	int		size ;
	char		*cp ;

	if (rpp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (sl < 0) sl = strlen(sp) ;

	size = (sl + 1) ;

#if	CF_STRLEN
	if ((rs = uc_malloc(size,&cp)) >= 0) {
	    int	cl ;
	    cl = strwcpy(cp,sp,sl) - cp ;
	    *rpp = cp ;
	    if ((sl-cl) > 10) {
		char	*ncp ;
		size = (cl+1) ;
		if ((rs = uc_malloc(size,&ncp)) >= 0) {
	    	    strncpy(ncp,cp,cl) ;
	    	    ncp[cl] = '\0' ;
	    	    *rpp = ncp ;
		}
		uc_free(cp) ;
	    }
	}
#else /* CF_STRLEN */
	if ((rs = uc_malloc(size,&cp)) >= 0) {
	    strncpy(cp,sp,sl) ;
	    cp[sl] = '\0' ;
	    *rpp = cp ;
	}
#endif /* CF_STRLEN */

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_mallocstrw) */


