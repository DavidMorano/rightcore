/* sbuf_termconseq */

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

	int sbuf_termconseq(op,n)
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
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sbuf.h"


/* local defines */

#ifndef	TERMCONSEQLEN
#define	TERMCONSEQLEN	84
#endif


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sbuf_termconseq(SBUF *sbp,int name,int a1,int a2)
{
	const int	termlen = TERMCONSEQLEN ;
	int		rs ;
	int		tl = 0 ;
	char		termbuf[TERMCONSEQLEN + 1] ;

	if (sbp == NULL) return SR_FAULT ;

	rs = termconseq(termbuf,termlen,name,a1,a2,-1,-1) ;
	tl = rs ;
	if (rs >= 0)
	    rs = sbuf_strw(sbp,termbuf,tl) ;

	return (rs >= 0) ? tl : rs ;
}
/* end subroutine (sbuf_termconseq) */


