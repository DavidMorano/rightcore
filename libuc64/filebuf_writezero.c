/* filebuf_writezero */

/* extra methods for the FILEBUF object */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We provide some extra small function for special circumstances.

	Synopsis:

	int filebuf_writezero(bp,size)
	FILEBUF		*bp ;
	int		size ;

	Arguments:

	bp		FILEBUF object pointer
	size		amount of zeros to write

	Returns:

	<0		error
	>=0		number of bytes written


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* exported variables */


/* local structures */


/* forward references */


/* local variables */

static cchar	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int filebuf_writezero(FILEBUF *fp,int size)
{
	int		rs = SR_OK ;
	int		ml ;
	int		rlen = size ;
	int		wlen = 0 ;

	while ((rs >= 0) && (rlen > 0)) {
	    ml = MIN(rlen,4) ;
	    rs = filebuf_write(fp,zerobuf,ml) ;
	    rlen -= rs ;
	    wlen += rs ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writezero) */


