/* main (testgetsystypenum) */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/utsname.h>
#include	<limits.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	45
#endif


/* external subroutines */

extern int	sfcookkey(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct utsname		u ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs ;

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

	if ((rs = u_uname(&u)) >= 0) {
	    char	tbuf[MAXNAMELEN+1] = { 0 } ;
	    char	nbuf[DIGBUFLEN+1] = { 0 } ;

	    rs = getsystypenum(tbuf,nbuf,u.sysname,u.release) ;

	    printf("rs=%d t=%s n=%s\n",rs,tbuf,nbuf) ;

	} /* end if (uname) */

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


