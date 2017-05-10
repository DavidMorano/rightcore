/* main (testsntmtime) */
/* lang=C99 */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<linefold.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"sntmtime.h"

#include	"defs.h"
#include	"config.h"

/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	WCHARLEN
#define	WCHARLEN	(10*LINEBUFLEN)
#endif

#define	PROGINFO	PROGINFO


/* external subroutines */

extern int	optvalue(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

/* forward references */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	TMTIME		tmt ;
	time_t		t = time(NULL) ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		cols = 0 ;
	const char	*termtype = getenv(VARTERM) ;
	const char	*pr = PCS ;
	const char	*cp ;

#if	CF_DEBUGS
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	memset(pip,0,sizeof(PROGINFO)) ;

	if ((rs >= 0) && (cols == 0)) {
	    if ((cp = getenv(VARCOLUMNS)) != NULL) {
	        rs = optvalue(cp,-1) ;
	        cols = rs ;
	    }
	}

	if ((rs >= 0) && (cols == 0)) {
	    cols = COLUMNS ;
	}

	pip->pr = pr ;
	pip->linelen = cols ;

/* go */

	if (rs >= 0) {
	if ((rs = tmtime_localtime(&tmt,t)) >= 0) {
	    const int	tlen = TIMEBUFLEN ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    const char	*fmt ;
	                int	ai ;
	                for (ai = 1 ; ai < argc ; ai += 1) {
			    fmt = argv[ai] ;
	                    rs = sntmtime(tbuf,tlen,&tmt,fmt) ;
			    if (rs >= 0)
				printf("%s\n",tbuf) ;
	                    if (rs < 0) break ;
	                } /* end for */
	} /* end if (tmtime_localtime) */
	} /* end if (ok) */

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


