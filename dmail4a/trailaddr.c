/* trailaddr */

/* find trailing address */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1997-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1997 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds a trailing address in a "stacked" address.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<localmisc.h>


/* exported subroutines */


const char *trailaddr(cchar addr[],int addrlen)
{
	int		al ;
	const char	*cp ;

	if (addr == NULL)
	    return NULL ;

	al = (addrlen >= 0) ? addrlen : strlen(addr) ;

/* remove trailing bangs */

	while ((al > 0) && (addr[al - 1] == '!'))
	    al -= 1 ;

#if	CF_DEBUGS
	debugprintf("trailaddr: addr=%t\n",addr,al) ;
#endif

	cp = addr + al ;
	while ((cp > addr) && (cp[-1] != '!'))
	    cp -= 1 ;

#if	CF_DEBUGS
	debugprintf("trailaddr: back=%t\n",cp,((addr + al) - cp)) ;
#endif

	if (cp == addr)
	    return cp ;

	if ((cp > addr) && (cp[-1] == '!'))
		cp -= 1 ;

	while ((cp > addr) && (cp[-1] != '!'))
	    cp -= 1 ;

#if	CF_DEBUGS
	debugprintf("trailaddr: back=%t\n",cp,((addr + al) - cp)) ;
#endif

	if (cp == addr)
	    return cp ;

	return cp ;
}
/* end subroutine (trailaddr) */


