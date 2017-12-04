/* getev */

/* get an environment variable by name */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is similar to 'getenv(3c)' except that the variable
	array (an array of pointers to strings) is explicitly passed by the
	caller.

	Synopsis:

	int getev(envv,kp,kl,rpp)
	const char	*envv[] ;
	const char	kp[] ;
	int		kl;
	const char	**rpp ;

	Arguments:

	envv		array of pointers to strings
	kp		name to search for
	kl		length of name string
	rpp		if not NULL a pointer to the found is stored

	Returns:

	>=0		length of found string (if any)
	SR_NOTFOUND	name was not found
	<0		bad


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>		/* extra types */


/* external subroutines */

extern int	matkeystr(cchar **,cchar *,int) ;


/* local variables */


/* exported subroutines */


int getev(cchar **envv,cchar *np,int nl,cchar **rpp)
{
	int		rs = SR_OK ;
	int		ei ;
	int		vl = 0 ;
	const char	*vp = NULL ;

	if (envv == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

	if ((ei = matkeystr(envv,np,nl)) >= 0) {
	    if ((vp = strchr(envv[ei],'=')) != NULL) {
	        vp += 1 ;
	        vl = strlen(vp) ;
	    }
	} else
	    rs = SR_NOTFOUND ;

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? vp : NULL ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (getev) */


