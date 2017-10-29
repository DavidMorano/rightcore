/* cmsghdr */

/* Conrol-Message-Header methods */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We perform some functions on a Control-Message-Header object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int cmsghdr_passed(CMSGHDR *cmp)
{
	const int	fdlen = sizeof(int) ;
	int		fd = -1 ;
	int		*ip = (int *) CMSG_DATA(cmp) ;
	int		f = TRUE ;
	f = f && (cmp->cmsg_level == SOL_SOCKET) ;
	f = f && (cmp->cmsg_len == CMSG_LEN(fdlen)) ;
	f = f && (cmp->cmsg_type == SCM_RIGHTS) && (ip != NULL) ;
	if (f) {
	    fd = *ip ;
	}
	return fd ;
}
/* end subroutine (cmsghdr_pass) */


