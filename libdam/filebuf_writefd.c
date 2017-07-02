/* filebuf_writefd */

/* mail supprt from the FILEBUF object */


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
#include	<string.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* write to the 'filebuf' file *from* the given FD */
int filebuf_writefd(FILEBUF *fbp,char *bp,int bl,int mfd,int len)
{
	int		rs = SR_OK ;
	int		rlen = len ;
	int		ml ;
	int		rl ;
	int		wlen = 0 ;

	while ((rs >= 0) && (rlen > 0)) {
	    ml = MIN(rlen,bl) ;
	    rs = u_read(mfd,bp,ml) ;
	    rl = rs ;
	    if (rs <= 0) break ;

	    rs = filebuf_write(fbp,bp,rl) ;
	    wlen += rs ;

	    rlen -= rl ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writefd) */


