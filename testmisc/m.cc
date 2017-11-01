/* main (timeout) */
/* lang=C++11 */

/* test the TIMEOUT facility */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#define	CF_DEBUGN	1		/* special debugging */


/* revision history:

	= 2017-10-12, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We (try to) test the TIMEOUT facility.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<stdio.h>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<set>
#include	<string>
#include	<ostream>
#include	<iostream>
#include	<vsystem.h>
#include	<ucmallreg.h>
#include	<localmisc.h>

#include	"timeout.h"


/* local defines */

#define	VARDEBUGFNAME	"TIMEOUT_DEBUGFILE"
#define	NDF		"uctimeout.deb"


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	uc_timeout(int,TIMEOUT *) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern "C" int	nprintf(cchar *,cchar *,...) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;

extern "C" void		uctimeout_fini() ;


/* local structures */


/* forward references */

static int	ourwake(void *,uint,int) ;


/* local variables */


/* exported subroutines */


/* ARHSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	TIMEOUT		to ;
	time_t		dt = time(NULL) ;
	time_t		wake ;
	const int	tval = 6 ;
#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif
	int		rs = SR_OK ;
	int		ex = 0 ;

#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	wake = (dt+(tval/2)) ;
	if ((rs = timeout_load(&to,wake,NULL,ourwake,0x5a5a,1)) >= 0) {
	    const int	cmd = timeoutcmd_set ;
	    if ((rs = uc_timeout(cmd,&to)) >= 0) {
	  	const int	id = rs ;

#if	CF_DEBUGN
		nprintf(NDF,"main: uc_timeout() rs=%d\n",rs) ;
#endif

		printf("id=%d\n",id) ;
	        sleep(tval) ;

		printf("done\n") ;
	    } /* end if (uc_timeout) */
	} /* end if (timeout_load) */

	uctimeout_fini() ;

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_wn: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"main: ret rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int ourwake(void *objp,uint tag,int arg)
{
	printf("int objp=%p tag=%u arg=%d\n",objp,tag,arg) ;
	return 0 ;
}
/* end subroutine (ourwake) */


