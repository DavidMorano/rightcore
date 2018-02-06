/* main (testbins) */
/* lang=C++11 */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testbins.x


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<forward_list>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<ostream>
#include	<iostream>
#include	<iomanip>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	OBUFLEN		(LINEBUFLEN*2)

#ifndef	PROGINFO
#define	PROGINFO	struct proginfo
#endif


/* namespaces */

using namespace	std ;


/* external subroutines */

extern "C" int	snwcpywidehdr(char *,int,const wchar_t *,int) ;
extern "C" int	isprintlatin(int) ;
extern "C" int	isprintbad(int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(const char *) ;
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugprinthex(const char *,int,const char *,int) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* forward references */

#if	CF_DEBUGS
static int	debugprintchars(cchar *,const wchar_t *,int) ;
#endif


/* local variables */

static const int 	values[] = {
	1,3,5,7, -1
} ;


/* exported subroutines */


/* ASRGUSED */
int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		ex = 0 ;
	const char	*pr = PCS ;
	const char	*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	memset(pip,0,sizeof(PROGINFO)) ;
	pip->pr = pr ;

/* go */

	if (rs >= 0) {
	    const int	svals[] = { 3, 0, 1, 9, 7, 5, 10 } ;
	    for (auto sch : svals) {
	        int	front = 0 ;
	        int	back = (nelem(values)-1) ;
	        int	ch = '¿' ;
	        int	i ;
		cout << "sch=" << sch << endl ;
	        i = ((back-front)/2) ;
#if	CF_DEBUGS
		{
		    const wchar_t	*w = (const wchar_t *) values ;
		    debugprintchars("main:",w,back) ;
		    debugprintf("main: sch=%d\n",sch) ;
		}
#endif
	        while (front < back) {
		    ch = values[i] ;
#if	CF_DEBUGS
		    debugprintf("main: top i=%u ch=%u\n",i,ch) ;
#endif
		    if (sch > ch) {
			front = (i+1) ;
		    } else if (sch < ch) {
			back = i ;
		    } else {
			break ;
		    }
#if	CF_DEBUGS
		    debugprintf("main: bot front=%u back=%u\n",front,back) ;
#endif
	            i = (front + ((back-front)/2)) ;
	        } /* end while */
#if	CF_DEBUGS
		debugprintf("main: front=%u\n",front) ;
		debugprintf("main: back=%u\n",back) ;
		debugprintf("main: i=%u ch=%u\n",i,ch) ;
#endif
		if (front < back) {
		    cout << "found i=" << i << " ch=" << ch << endl ;
		} else {
		    cout << "not found" << endl ;
		}
	    } /* end for */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("main: done rs=%d\n",rs) ;
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

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


#if	CF_DEBUGS
static int debugprintchars(cchar *id,const wchar_t *wbuf,int wlen)
{
	int	i ;
	for (i = 0 ; i < wlen ; i += 1) {
	    const int	ch = wbuf[i] ;
	    debugprintf("main/%s: wc[%02u]=%08x\n",id,i,ch) ;
	}
	return 0 ;
}
/* end subroutine (debugprintchars) */
#endif /* CF_DEBUGS */

