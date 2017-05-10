/* main (testexpcook) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"expcook.h"

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


struct cookpair {
	const char	*k ;
	const char	*v ;
} ;

static const struct cookpair	pairs[] = {
	{ "R", "/usr/add-on/pcs" },
	{ "N", "nodename" },
	{ "U", "username" },
	{ "tz", "time-zone" },
	{ "hwserial", "hardware-serial" },
	{ NULL, NULL }
} ;


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	EXPCOOK		ec ;
	EXPCOOK_CUR	cur ;

	const int	klen = KBUFLEN ;
	const int	wch = MKCHAR('¿') ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs ;
	int	cl ;

	const char	*s = "here %R is %{tz} thing %{} %{junker} end" ;
	const char	*cp ;

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

	cl = sfcookkey(s,-1,&cp) ;

	printf("cl=%d cp={%p}\n",cl,cp) ;

	if (cl >= 0)
	strdcpy1w(kbuf,klen,cp,cl) ;

	printf("k=%s\n",kbuf) ;

/* start in */

	if ((rs = expcook_start(&ec)) >= 0) {
	    int		i ;
	    const char	*k, *v ;

	    for (i = 0 ; (rs >= 0) && (pairs[i].k != NULL) ; i += 1) {
		k = pairs[i].k ;
		v = pairs[i].v ;
		rs = expcook_add(&ec,k,v,-1) ;
	    }

	    if (rs >= 0) {
		if ((rs = expcook_curbegin(&ec,&cur)) >= 0) {
		    const int	rlen = VBUFLEN ;
		    char	rbuf[VBUFLEN+1] ;
	    	    int		rl ;
	            while ((rl = expcook_enum(&ec,&cur,rbuf,rlen)) >= 0) {

			printf("pair> %s\n",rbuf) ;

		    } /* end while */
		    expcook_curend(&ec,&cur) ;
		} /* end if (cursor) */
	    } /* end if */

	    if (rs >= 0) {
		const int	rlen = VBUFLEN ;
		char		rbuf[VBUFLEN+1] ;
		if ((rs = expcook_exp(&ec,wch,rbuf,rlen,s,-1)) >= 0) {

		    printf("exp> %s\n",rbuf) ;

		} /* end if (expand) */
	    } /* end if (substitutions) */

	    expcook_finish(&ec) ;
	} /* end if (expandcookie) */

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


