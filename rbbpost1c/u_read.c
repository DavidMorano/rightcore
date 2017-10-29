/* u_read */

/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services (RNS).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */

#define	TO_NOLCK	10


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_read(fd,ubuf,ulen)
int		fd ;
void		*ubuf ;
int		ulen ;
{
	size_t	rlen = ulen ;

	int	rs ;
	int	to_nolck = TO_NOLCK ;


#if	CF_DEBUGS
	debugprintf("u_read: ent fd=%d ulen=%d\n",fd,ulen) ;
#endif

again:
	if ((rs = read(fd,ubuf,rlen)) < 0) 
	    rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("u_read: rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    switch (rs) {

	    case SR_INTR:
	        goto again ;

	    case SR_NOLCK:
	        if (to_nolck-- <= 0) break ;
	        msleep(1000) ;
	        goto again ;

	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (u_read) */



