/* testnprint */
/* lang=C89 */

#define	CF_DEBUGS	1
#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<localmisc.h>

#ifndef	COLUMNS
#define	COLUMNS	80
#endif

#define	VARDEBUGFNAME	"TESTNPRINT_DEBUGFILE"

#define	NDF		"testnprint.deb"

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	nprinthexblock(cchar *,cchar *,int,cchar *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(cchar **,cchar *) ;

int main(int argc,cchar **argv,cchar **envv)
{
	const int	cols = COLUMNS ;
	int		rs ;
	cchar		*msg = "here/nis/hello" ;

#if	CF_DEBUGS
	cchar		*cp ;
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = nprintf(NDF,"main: msg=%s\n",msg) ;

#if	CF_DEBUGS
	    debugprintf("main: nprintf() rs=%d\n",rs) ;
#endif

	rs = nprinthexblock(NDF,"main: ",cols,msg,-1) ;

#if	CF_DEBUGS
	    debugprintf("main: nprinthexblock() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


