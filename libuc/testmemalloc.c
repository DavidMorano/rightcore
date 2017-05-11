/* testmemalloc (C89) */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory allocations */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<fcntl.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"testmemalloc.h"

#ifndef	FD_STDOUT
#define	FD_STDOUT	1
#endif

/* external subroutines */

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	uint		mo_start = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		size ;
	const char	*cp ;
	char		*p ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	size = 3 ;
	rs1 = uc_malloc(size,&p) ;
	fprintf(stderr,"main: uc_malloc() rs=%d\n",rs1) ;

	rs1 = uc_free(p) ;
	fprintf(stderr,"main: uc_free() rs=%d\n",rs1) ;

	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("main: exiting rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


