/* testlastlog */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#include	<envstandards.h>
#include	<sys/types.h>
#include	<lastlog.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<localmisc.h>

#define	VARDEBUGFNAME	"TESTLASTLOG_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs ;
	int	rs1 ;

	const char	*llfname = "/var/adm/lastlog" ;


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

	if ((rs = u_open(llfname,O_WRONLY,0666)) >= 0) {
	    offset_t	eoff ;
	    const int	llsize = sizeof(struct lastlog) ;
	    int		fd = rs ;
	    char	lbuf[sizeof(struct lastlog)] ;
	    eoff = (215*llsize) ;
	    if ((rs = u_seek(fd,eoff,SEEK_SET)) >= 0) {
		memset(lbuf,0,llsize) ;
		rs = u_write(fd,lbuf,llsize) ;
	    }
	    u_close(fd) ;
	} /* end if (file-open) */

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


