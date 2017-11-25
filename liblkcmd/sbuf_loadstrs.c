/* sbuf_loadstrs */

/* load a set of strings into the SBUF object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This SBUF method stores a set of strings into the object.

	Synopsis:

	int sbuf_loadstrs(op,sa)
	SBUF		*op ;
	cchar		**sa ;

	Arguments:

	op		pointer to the buffer object
	sa		array of strings

	Returns:

	<0		error
	>=0		amount of new space used by the newly stored item
			(not including any possible trailing NUL characters)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"sbuf.h"


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sbuf_loadstrs(SBUF *sbp,cchar **spp)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (spp != NULL) {
	    while (*spp != NULL) {
	        c += 1 ;
	        rs = sbuf_strw(sbp,*spp,-1) ;
	        spp += 1 ;
	        if (rs < 0) break ;
	    } /* end while */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sbuf_loadstrs) */


