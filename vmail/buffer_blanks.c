/* buffer-extras */

/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-06-24, David A­D­ Morano
	This was separated out from TERMSTR.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Store a given number of blanks (space characters) into the "buffer."

	Synopsis:

	int buffer_blanks(op,n)
	BUFFER		*op ;
	int		n ;

	Arguments:

	op		object pointer
	n		<n> blanks

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<buffer.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strnnlen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char	blanks[] = "        " ;


/* exported subroutines */


int buffer_blanks(BUFFER *bp,int n)
{
	const int	nblanks = sizeof(blanks) ;
	int		rs = SR_OK ;
	int		m ;
	int		len = 0 ;

	while ((rs >= 0) && (n > 0)) {
	    m = MIN(n,nblanks) ;
	    rs = buffer_strw(bp,blanks,m) ;
	    n -= m ;
	    len += rs ;
	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (buffer_blanks) */


int buffer_backs(BUFFER *bp,int n)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	while ((rs >= 0) && (n-- > 0)) {
	    rs = buffer_char(bp,CH_BS) ;
	    len += rs ;
	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (buffer_backs) */


