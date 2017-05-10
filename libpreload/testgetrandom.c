/* testgetrandom */
/* lang=C89 */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/random.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<localmisc.h>

/* local defines */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	RBUFLEN		20

#define	VARDEBUGFNAME	"TESTUCOPEN_DEBUGFILE"
#define	VARCOLUMNS	"COLUMNS"

extern int	cfdeci(cchar *,int,int *) ;
extern int	fbwrite(FILE *,const void *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(const char **,const char *) ;

extern char	*timestr_logz(time_t,char *) ;

/* forward references */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;
	int		cols = COLUMNS ;
	cchar		*cp ;

#if	CF_DEBUGS
	{
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs1 = debugopen(cp) ;
	        debugprintf("main: DEB fd=%u\n",rs1) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((cp = getourenv(envv,VARCOLUMNS)) != NULL) {
	     int	v ;
	     rs = cfdeci(cp,-1,&v) ;
	     cols = v ;
	}

	if (rs >= 0) {
	    const int	rlen = RBUFLEN ;
	    int		rc = 0 ;
	    cchar	*pn = "testgetrandom" ;
	    char	rbuf[RBUFLEN+1] ;
	    printf("random> ") ;
	    if ((rc = getrandom(rbuf,rlen,0)) >= 0) {
		const int	rl = rc ;
		int		ch ;
		int		i ;
#if	CF_DEBUGS
	        debugprinthexblock(pn,cols,rbuf,rl) ;
#endif
		for (i < 0 ; i < rl ; i += 1) {
		    ch = MKCHAR(rbuf[i]) ;
		    printf(" %02x",ch) ;
		}
		printf("\n") ;
	    } /* end if */
#if	CF_DEBUGS
	    debugprintf("main: getrandom() rc=%d\n",rc) ;
#endif
	} /* end if (arguments) */

	if (rs < 0)
	printf("failure (%d)\n",rs) ;

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


