/* main (testigcd) */
/* lang=C99 */

#define	CF_DEBUGS	1		/* compile-time debugging */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	The the |igcd(3dam)| function.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40
#endif

#define	MAXTRY		20		/* maximum value for each element */

#define	VARDEBUGFNAME	"TESTISHEX_DEBUGFILE"

extern int	igcd(int,int) ;

/* external subroutines */

extern int	optvalue(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(const char **,const char *) ;


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	int		rs = SR_OK ;
	int		ex = 0 ;
	int		maxtry = MAXTRY ;
	int		i, j ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if ((rs >= 0) && (argc > 1)) {
	    if ((argv[1] != NULL) && (argv[1][0] != '\0')) {
		cchar	*ap = argv[1] ;
		rs = optvalue(ap,-1) ;
		maxtry = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("main: maxtry=%u\n",maxtry) ;
#endif

	if (rs >= 0) {
	    for (i = 1 ; i < maxtry ; i += 1) {
	        for (j = 1 ; j < maxtry ; j += 1) {
		    const int	g = igcd(i,j) ;
		    printf("g(%2u,%2u)=%u\n",i,j,g) ;
	        }
	    }
	}

#if	CF_DEBUGS
	debugprintf("main: done rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


