/* testopendialer */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<filebuf.h>
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

#define	VARDEBUGFNAME	"TESTOPENDIALER_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

extern char	*timestr_logz(time_t,char *) ;

/* forward references */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;


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
	    const int	llen = LINEBUFLEN ;
	    int		ai ;
	    char	lbuf[LINEBUFLEN+1] ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	        const char	*fn = argv[ai] ;
	        const int	of = O_RDONLY ;
#if	CF_DEBUGS
	        debugprintf("main: fn=%s\n",fn) ;
#endif
	        if ((rs = uc_open(fn,of,0666)) >= 0) {
	            FILEBUF	b ;
	            const int	fo = (of | O_NETWORK) ;
	            int		fd = rs ;
#if	CF_DEBUGS
	            debugprintf("main: uc_open() rs=%d\n",rs1) ;
#endif
	            if ((rs = filebuf_start(&b,fd,0L,0,fo)) >= 0) {
	                const int	to = 5 ;
	                while ((rs = filebuf_read(&b,lbuf,llen,to)) > 0) {
	                    int	len = rs ;
#if	CF_DEBUGS
	                    debugprintf("main: readline() len=%d\n",len) ;
#endif
	                    fbwrite(stdout,lbuf,len) ;
	                    if (rs < 0) break ;
	                } /* end while */
	                rs1 = filebuf_finish(&b) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (filebuf) */
#if	CF_DEBUGS
	            debugprintf("main: readline-out rs=%d\n",rs) ;
#endif
	            rs1 = u_close(fd) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
	            fbprintf(stdout,"not_found fn=%s (%d)\n",fn,rs) ;
	        }
#if	CF_DEBUGS
	        debugprintf("main: uc_open-out rs=%d\n",rs1) ;
#endif
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (arguments) */

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
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


