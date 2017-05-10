/* readignore */

/* read data while ignoring it */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Read a specified amount of data ignoring it.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int readignore(int fd,offset_t amount)
{
	const int	rlen = getpagesize() ;
	int		rs ;
	char		*rbuf ;
	if ((rs = uc_malloc(rlen,&rbuf)) >= 0) {
	    int		ml ;
	    while ((rs >= 0) && (amount > 0)) {
		ml = (int) MIN(rlen,amount) ;
		rs = u_read(fd,rbuf,ml) ;
	 	amount -= rs ;
	    } /* end while */
	    uc_free(rbuf) ;
	} /* end if (m-a-f) */
	return rs ;
}
/* end subroutine (readignore) */


