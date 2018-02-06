/* sbuf_blanks */

/* extra method for the storage buffer (SBuf) object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine stores a user-specified number of blank characters into
        the buffer.

	Synopsis:

	int sbuf_blanks(op,n)
	SBUF		*op ;
	int		n ;

	Arguments:

	op		pointer to the buffer object
	n		number of blanks to store

	Returns:

	>=0		amount of new space used by the newly stored item
			(not including any possible trailing NUL characters)
	<0		error


*******************************************************************************/


#define	SBUF_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sbuf.h"


/* local defines */

#define	SBUF_NBLANKS		20


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */

static const char	blanks[SBUF_NBLANKS] = "                    " ;


/* exported subroutines */


int sbuf_blanks(SBUF *sbp,int n)
{
	int		rs = SR_OK ;
	int		ml ;
	int		len = 0 ;

	while ((rs >= 0) && (len < n)) {
	    ml = MIN(SBUF_NBLANKS,(n-len)) ;
	    rs = sbuf_strw(sbp,blanks,ml) ;
	    len += ml ;
	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_blanks) */


