/* mkcexsync */

/* make the synchronization string used for CEX */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1992-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1992 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates the synchronization string sequence for use by
	CEX.

	Synopsis:

	int mkcexsync(rbuf,rlen)
	char	rbuf[] ;
	int	rlen ;

	Arguments:

	rbuf		host to dial to
	rlen		length of buffer (really just a check)

	Returns:

	>=0		length of resulting sequence
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"mkcexsync.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local typedefs */

#if	defined(IRIX) && (! defined(TYPEDEF_INADDRT))
#define	TYPEDEF_INADDRT	1

typedef unsigned int	in_addr_t ;

#endif


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkcexsync(char *rbuf,int rlen)
{
	const int	leaderlen = (rlen - MKCEXSYNC_FINLEN) ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rlen >= MKCEXSYNC_REQLEN) {
	    int	j ;

	    for (j = (leaderlen-1) ; j >= 0 ; j -= 1) {
	        rbuf[i] = (i & 1) ;
	        i += 1 ;
	    }

	    for (j = 0 ; j < MKCEXSYNC_FINLEN ; j += 1) {
	        rbuf[i++] = CH_SYNC ;
	    }

	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkcexsync) */


