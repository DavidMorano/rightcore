/* getenv3 */

/* get the value of an environment variable */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This subroutine was written from scratch. There are (or may) be some
        other standard ones floating around like it but I could not find one
        that was exactly what I needed (sigh).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is like 'getenv(3c)' except that it also takes an
        argument that specifies the length of the environment variable string to
        look up in the environment array.

        This subroutine also returns a pointer to the entire string (key and
        value pair) as it is found (if found) in the environment array.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<localmisc.h>


/* external variables */

extern cchar	**environ ;


/* exported subroutines */


const char *getenv3(cchar *name,int namelen,cchar **epp)
{
	int		len, i ;
	cchar		*np ;
	cchar		**p, *cp ;
	cchar		*rp = NULL ;

	if (environ == NULL) return NULL ;
	if (name == NULL) return NULL ;

	if (namelen < 0)
	    namelen = strlen(name) ;

	for (np = name ; 
	    (np < (name + namelen)) && *np && (*np != '=') ; np += 1) ;

	len = np - name ;
	for (p = environ ; *p != NULL ; p += 1) {

	    cp = *p ;
	    for (np = name, i = len ; i && *cp ; i -= 1) {
	        if (*cp++ != *np++) break ;
	    }

	    if ((i == 0) && (*cp++ == '=')) {
	        if (epp != NULL) *epp = *p ;
		rp = cp ;
		break ;
	    } /* end if */

	} /* end for */

	return rp ;
}
/* end subroutine (getenv3) */


