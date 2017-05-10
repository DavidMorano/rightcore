/* testopenweather */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdarg.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<fsdir.h>
#include	<openpcsdircache.h>
#include	<filebuf.h>
#include	<localmisc.h>

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#ifndef	UEBUFLEN
#define	UEBUFLEN	UTMPACCENT_BUFLEN
#endif

#ifndef FILEBUF_RCNET
#define	FILEBUF_RCNET	4		/* read-count for network */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	VARDEBUGFNAME	"TESTOPENWEATHER_DEBUGFILE"

/* external subroutines */

extern int	openweather(const char *,const char *,int,int) ;
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

static int procweather(int,int) ;

/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{
	const char	*pr = getourenv(envv,VARPRLOCAL) ;

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

#if	CF_DEBUGS
	        debugprintf("main: pr=%s\n",pr) ;
#endif

	if (argv != NULL) {
	    const mode_t	om = 0666 ;
	    const int	of = O_RDONLY ;
	    const int	to = -1 ;
	    const int	llen = LINEBUFLEN ;
	    int		ai ;
	    char	lbuf[LINEBUFLEN+1] ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	        const char	*ws = argv[ai] ;
#if	CF_DEBUGS
	        debugprintf("main: ws=%s\n",ws) ;
#endif
	        if ((rs1 = openweather(pr,ws,of,to)) >= 0) {
	            struct ustat	sb ;
	            int		fd = rs1 ;
#if	CF_DEBUGS
	            debugprintf("main: openweather() rs=%d\n",rs1) ;
#endif
	            if ((rs = u_fstat(fd,&sb)) >= 0) {
#if	CF_DEBUGS
	            debugprintf("main: mode=\\x%08x\n",sb.st_mode) ;
#endif

			rs = procweather(fd,of) ;
		    }
	            
	            u_close(fd) ;
	        } else if (rs1 == SR_NOTFOUND) {
	            rs = SR_OK ;
	            printf("not_found ws=%s (%d)\n",ws,rs1) ;
	        }
#if	CF_DEBUGS
	        debugprintf("main: openweather-out rs=%d\n",rs1) ;
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


static int procweather(int fd,int of)
{
	FILEBUF	b ;
	const int	fo = (of | O_NETWORK) ;
	int	rs ;
#if	CF_DEBUGS
	debugprintf("main/dumpfile: entered\n") ;
#endif
	if ((rs = filebuf_start(&b,fd,0L,0,fo)) >= 0) {
	    const int	to = 0 ;
	    const int	llen = LINEBUFLEN ;
	    int		li ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	        int	len = rs ;
#if	CF_DEBUGS
	        debugprintf("main/dumpfile: readline() len=%d\n",len) ;
#endif
	        li = strlinelen(lbuf,len,70) ;
	        lbuf[li] = '\0' ;
	        printf("l=>%s<\n",lbuf) ;
	        if (rs < 0) break ;
	    } /* end while */
	    filebuf_finish(&b) ;
	} /* end if (filebuf) */

#if	CF_DEBUGS
	debugprintf("main/dumpfile: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procweather) */


