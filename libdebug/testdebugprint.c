/* testdebugprint */
/* lang=C89 */

#define	CF_DEBUGS	1
#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>

#define	VARDEBUGFNAME	"TESTDEBUGPRINT_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

int main(int argc,const char **argv,const char **envv)
{
	int		rs ;
	int		ex = 0 ;
	cchar		*msg = "here\nis\033" ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = debugprintf("main: msg=>%s<\n",msg) ;

	if (rs < 0) {
	    debugprintf("main: rs=%d\n",rs) ;
	}

#if	CF_DEBUGS
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


