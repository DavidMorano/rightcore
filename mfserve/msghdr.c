/* msghdr */

/* messsage-header methods */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Message-Header support.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/uio.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* local variables */


/* exported subroutines */


int msghdr_size(MSGHDR *mhp)
{
	struct iovec	*vlp = mhp->msg_iov ;
	int		vll = mhp->msg_iovlen ;
	int		i ;
	int		size = 0 ;
	for (i = 0 ; i < vll ; i += 1) {
	   size += vlp->iov_len ;
	}
	return size ;
}
/* end subroutine (msghdr_size) */


