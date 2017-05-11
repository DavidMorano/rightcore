/* pt_atfork */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int pt_atfork(void (*b)(),void (*ap)(),void (*ac)())
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("pt_atfork: ent\n") ;
#endif

	repeat {
	    if (pthread_atfork(b,ap,ac) != 0) rs = (- errno) ;
	} until (rs != SR_INTR) ;

#if	CF_DEBUGS
	debugprintf("pt_atfork: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pt_atfork) */


