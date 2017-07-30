/* testminmax */
/* lang=C99 */

#define	CF_DEBUGS	1

/* revision history:

	= 1998-07-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	<vecpstr.h>
#include	<cthex.h>
#include	<vsystem.h>

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40
#endif

#define	VARDEBUGFNAME	"TESTISHEX_DEBUGFILE"


extern int	min(int,int) ;
extern int	max(int,int) ;

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar 	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;

int main(int argc,cchar **argv,cchar **envv)
{
	const int	n = (100*1024*1024) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		prev = 0 ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	for (i = 0 ; i < n ; i += 1) {
	    int	v = random() ;
	    if (min(prev,v)) {
		rs = 1 ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


