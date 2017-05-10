/* filebuf_writeblanks */

/* write blacks to a FILEBUF object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Write a specified number of blanks to a FILEBUF object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"filebuf.h"


/* local defines */

#undef	NBLANKS
#define	NBLANKS		8


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */


/* local variables */

static const char	blanks[NBLANKS] = "        " ;


/* exported subroutines */


int filebuf_writeblanks(FILEBUF *fbp,int n)
{
	int		rs = SR_OK ;
	int		ml ;
	int		wlen = 0 ;

	while ((rs >= 0) && (wlen < n)) {
	    ml = MIN((n-wlen),NBLANKS) ;
	    rs = filebuf_write(fbp,blanks,ml) ;
	    wlen += rs ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writeblanks) */


