/* main (testvecsorthand) */
/* lang=C++11 */

/* sorted vector testing */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<string.h>
#include	<stdlib.h>

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
#include	<bfile.h>
#include	<field.h>
#include	<vechand.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"vecsorthand.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* name spaces */

using namespace	std ;


/* typedefs */


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* forward references */

static int	mainfins(vecsorthand *) ;
static int	mainadd(vecsorthand *,int) ;

static int	ourcmp(const void *,const void *) ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	VECSORTHAND	ourlist ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = 0 ;
	cchar		*progname ;


#if	CF_DEBUGS
	{
	cchar		*cp ;
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
	}
#endif /* CF_DEBUGS */

	progname = argv[0] ;

	if ((rs = vecsorthand_start(&ourlist,0,ourcmp)) >= 0) {
	    const void	*ep ;
	    int		i ;
	    int		v ;

	    rs = mainadd(&ourlist,1) ;
#if	CF_DEBUGS
	    debugprintf("main: mainadd() rs=%d\n",rs) ;
#endif
	    mainadd(&ourlist,2) ;
	    mainadd(&ourlist,3) ;

#if	CF_DEBUGS
	    debugprintf("main: getting\n") ;
#endif

	    for (i = 0 ; vecsorthand_get(&ourlist,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
		    int	*ip = (int *) ep ;
		    v = *ip ;
		    cout << ' ' << v ;
	        }
	    }
	    cout << endl ;

	    rs1 = mainfins(&ourlist) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = vecsorthand_finish(&ourlist) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecsorthand) */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int mainadd(vecsorthand *vlp,int v)
{
	const int	esize = sizeof(int) ;
	int		rs ;
	int		*ip ;
	if ((rs = uc_malloc(esize,&ip)) >= 0) {
	    *ip = v ;
	    rs = vecsorthand_add(vlp,ip) ;
	}
	return rs ;
}
/* end subroutine (mainadd) */


static int mainfins(vecsorthand *vlp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		*ep ;

	for (i = 0 ; vecsorthand_get(vlp,i,&ep) >= 0 ; i += 1) {
	    rs1 = uc_free(ep) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (mainfins) */


static int ourcmp(const void *a1p,const void *a2p)
{
	int		**e1pp = (int **) a1p ;
	int		**e2pp = (int **) a2p ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
		    int	*i1p = (int *) *e1pp ;
		    int	*i2p = (int *) *e2pp ;
	            rc = (*i1p - *i2p) ;
	        } else
		    rc = -1 ;
	    } else
		rc = 1 ;
	}
	return rc ;

}
/* end subroutine (ourcmp) */


