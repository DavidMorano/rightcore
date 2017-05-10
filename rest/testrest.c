/* testrest */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#ifndef	UEBUFLEN
#define	UEBUFLEN	UTMPACCENT_BUFLEN
#endif

#ifndef FILEBUF_RCNET
#define	FILEBUF_RCNET	4		/* read-count for network */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	VARDEBUGFNAME	"TESTREST_DEBUGFILE"

extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

extern char	*timestr_logz(time_t,char *) ;

/* forward references */

static int fileclear(int) ;


/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;
	int	ex = EX_INFO ;
	int	n = 1000 ;


#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if (argv != NULL) {
	    int		asize ;
	    int		ai ;
	    int		i ;
	    int		ac = MIN(argc,2) ;
	    const char	**av ;
	    asize = ((ac+1)*sizeof(const char *)) ;
	    if ((rs = uc_malloc(asize,&av)) >= 0) {

		av[0] = "rest" ;
		for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
		    av[ai] = argv[ai] ;
		}
		av[ai] = NULL ;

	        for (i = 0 ; i < n ; i += 1) {
		    fileclear(2) ;

		    ex = p_rest(ai,av,envv,NULL) ;

		    if (ex != EX_OK) break ;
	        } /* end for */

#if	CF_DEBUGS
	        debugprintf("main: while-out rs=%d\n",rs1) ;
#endif

	    } /* end if (memory-allocation) */

	} /* end if (argv) */

#if	CF_DEBUGS
	debugprintf("main: all-out rs=%d\n",rs) ;
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

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int fileclear(int fd) 
{
	int	rs ;
	if ((rs = u_rewind(fd)) >= 0)
	    rs = uc_ftruncate(fd,0L) ;
	return rs ;
}
/* end subroutine (fileclear) */


