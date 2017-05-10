/* main (testattach) */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"


#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

extern int	sfcookkey(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	const int	klen = KBUFLEN ;
	const int	wch = MKCHAR('¿') ;

	uint	mo_start = 0 ;

	int	rs ;
	int	cl ;

	const char	*s = "here %R is %{tz} thing %{} %{junker} end" ;
	const char	*cp ;
	const char	*ad = "ad" ;
	const char	*mnt = "mnt" ;

	char	kbuf[KBUFLEN+1] = { 0 } ;


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

	if ((rs = u_open(ad,O_RDONLY,0666)) >= 0) {
	    int	dfd = rs ;

#if	CF_DEBUGS
	    debugprintf("main: u_open() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	    if ((rs = uc_fattach(dfd,mnt)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: uc_fattach() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */
		sleep(60) ;
	    }

#if	CF_DEBUGS
	    debugprintf("main: uc_fattach rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	    u_close(dfd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	    debugprintf("main: exiting rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

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


