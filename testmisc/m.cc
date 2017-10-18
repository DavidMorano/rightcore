/* main (timeout) */
/* lang=C++11 */

/* test the TIMEOUT facility */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */


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
#include	<iostream>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"timeout.h"


/* local defines */

#define		VARDEBUGFNAME	"TESTTIMEOUT_DEBUGFILE"


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern int	uc_timeout(int,TIMEOUT *) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


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

	wake = (dt+3) ;
	if ((rs = timeout_init(&to,wake,NULL,ourwake,0,0)) >= 0) {
	    const int	cmd = timeoutcmd_set ;
	    if ((rs = uc_timeout(cmd,&to)) >= 0) {
	  	const int	id = rs ;
		printf("id=%d\n",id) ;

	        sleep(10) ;

		printf("done\n") ;
	    } /* end if (uc_timeout) */
	} /* end if (timeout_init) */

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
	printf("int tag=%u arg=%d\n",tag,arg) ;
	return 0 ;
}
/* end subroutine (ourwake) */


