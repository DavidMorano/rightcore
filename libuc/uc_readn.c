/* uc_readn */

/* interface component for UNIX® library-3c */
/* read a fixed number of bytes */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	MAXEOF		4		/* maximum consecutive EOFs */


/* external subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_readn(fd,abuf,alen)
int	fd ;
void	*abuf ;
int	alen ;
{
	int	rs = SR_OK ;
	int	len ;
	int	alenr = alen ;
	int	c = 0 ;
	int	rlen = 0 ;

	char	*abp = (char *) abuf ;


	if (fd < 0)
	    return SR_BADF ;

	if (abuf == NULL)
	    return SR_FAULT ;

	if (alen < 0)
	    return SR_INVALID ;

	while ((rs >= 0) && (alenr > 0)) {

	    rs = u_read(fd,abp,alenr) ;
	    len = rs ;
	    if (rs > 0) {
		abp += len ;
		rlen += len ;
		alenr -= len ;
	    } else if (rs == 0) {
		c += 1 ;
		if (c >= MAXEOF)
		    break ;
	    }

	} /* end while */

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (uc_readn) */


