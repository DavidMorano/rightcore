/* strdomain */

/* return a pointer to the domain part (last two components) of a hostname */
/* last modified %G% version %I% */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns a pointer to the last two domain components of
	a hostname.

	Synopsis:

	const char *strdomain(name)
	const char	name[] ;

	Arguments:

	name		hostname to test

	Returns:

	<value>		a pointer to the last two components
	NULL		there were not two components


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


const char *strdomain2(cchar *name)
{
	int		nlen ;
	const char	*cp ;
	char		*rp = NULL ;

	if (name == NULL) return NULL ;

	nlen = strlen(name) ;

	while ((nlen > 0) && (name[nlen - 1] == '.')) {
	    nlen -= 1 ;
	}

	cp = name + nlen - 1 ;
	while ((cp >= name) && (*cp != '.')) {
	    cp -= 1 ;
	}

	if (cp != name) {
	    cp -= 1 ;
	    while ((cp >= name) && (*cp != '.')) {
	        cp -= 1 ;
	    }
	    if (cp != name) {
	        rp = (char *) (cp+1) ;
	    } else {
	        rp = (char *) cp ;
	    }
	} else {
	    rp = (char *) cp ;
	}

	return rp ;
}
/* end subroutine (strdomain2) */


const char *strdomain(cchar *name)
{

	return strdomain2(name) ;
}
/* end subroutine (strdomain) */


