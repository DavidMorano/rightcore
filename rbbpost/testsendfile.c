/* testsendfile */
/* lang=C89 */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<opendial.h>
#include	<localmisc.h>

#define	VARDEBUGFNAME	"TESTSENDFILE_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

static int procfile(int,const char *) ;

int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	const int	d = OPENDIAL_DTCPNLS ;
	const int	opts = 0 ;
	const int	to = -1 ;
	const int	af = AF_UNSPEC ;

	int	rs ;
	int	rs1 ;

	const char	*hs = "localhost" ;
	const char	*ps = NULL ;
	const char	*svc = "dump" ;
	const char	**av = NULL ;
	const char	**ev = NULL ;

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

	if ((rs = opendial(d,af,hs,ps,svc,av,ev,to,opts)) >= 0) {
	    int	fd = rs ;

	    sleep(2) ;

	    if (argv != NULL) {
	        int	ai ;
	        const char	*fn ;
	        for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	            fn = argv[ai] ;
	            rs = procfile(fd,fn) ;
#if	CF_DEBUGS
	            debugprintf("main: procfile() rs=%d\n",rs) ;
#endif
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("main: done rs=%d\n",rs) ;
#endif

	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open-dial) */

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

static int procfile(int wfd,const char *fn)
{
	int	rs = SR_OK ;
	int	wlen = 0 ;

	if ((rs = uc_open(fn,O_RDONLY,0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		fd = rs ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = u_read(fd,lbuf,llen)) > 0) {
		int	len = rs ;
	        rs = u_write(wfd,lbuf,len) ;
		wlen += rs ;
		if (rs < 0) break ;
	    } /* end while */
	} /* end if (file-open) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (main) */


