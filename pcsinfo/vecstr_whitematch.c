/* vecstr_whitematch */


#define	CF_DEBUGS	0


/* revision history:

	= 2004-05-27, David A­D­ Morano
        This is a quickie replacement for a straight search. We add the ability
        to do a domain-only match.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine does a string search but it will also match when just a
        domain is in the existing list (and the given domain matches it).


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"address.h"


/* forward references */

static int	cmpaddr() ;


/* exported subroutines */


int vecstr_whitematch(lp,fp)
vecstr		*lp ;
const char	*fp ;
{
	int	rs = SR_NOTFOUND ;


	if (strpbrk(fp,"@!") != NULL)
		rs = vecstr_search(lp,fp,cmpaddr,NULL) ;

	return rs ;
}
/* end subroutine (vecstr_whitematch) */


/* local subroutines */


static int cmpaddr(e1pp,e2pp)
const char	**e1pp, **e2pp ;
{
	int	rc ;

	const char	*e1p, *e2p ;

	char		*h1p, *h2p ;


	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;

	h1p = strchr(e1p,'@') ;

	h2p = strchr(e2p,'@') ;

	if ((h1p == NULL) || (h2p == NULL)) {

		if (h1p == NULL)
			rc = strcmp(e1p,(h2p + 1)) ;

		else
			rc = strcmp((h1p + 1),e2p) ;

	} else
		rc = strcmp(e1p,e2p) ;

	return rc ;
}
/* end subroutine (cmpaddr) */



