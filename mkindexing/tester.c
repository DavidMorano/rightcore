/* tester */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<localmisc.h>

#include	"upt.h"
#include	"defs.h"
#include	"config.h"


/* local defines */

#ifndef	VARLOCAL
#define	VARLOCAL	"LOCAL"
#endif

#ifndef	PRLOCAL
#define	PRLOCAL	"/usr/add-on/local"
#endif

extern int	ncpu(const char *) ;


/* exported subroutines */


int main()
{
	int	rs1 ;
	int	npar = 3 ;
	int	n = 0 ;

	const char	*pr = PRLOCAL ;

	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs1 = uptgetconcurrency() ;
	fprintf(stdout,"concurrency=%d\n",rs1) ;

	uptsetconcurrency(npar) ;
	rs1 = uptgetconcurrency() ;
	fprintf(stdout,"concurrency=%d\n",rs1) ;

	rs1 = ncpu(pr) ;

	fprintf(stdout,"pr=%s rs1=%d\n",pr,rs1) ;

	fflush(stdout) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


