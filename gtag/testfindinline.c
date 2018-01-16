/* testfindinline */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#include	<envstandards.h>
#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"findinline.h"

#define	VARDEBUGFNAME	"TESTFINDINLINE_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	FINDINLINE	fi ;

	bfile	ofile, *ofp = &ofile ;
	bfile	ifile, *ifp = &ifile ;

	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*sp ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs ;
	int	rs1 ;
	int	sl ;

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

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
		ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

#ifdef	COMMENT
	if (argv != NULL) {
	    int	ai ;
	    for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {


#if	CF_DEBUGS
	        debugprintf("main: findinline() rs=%d\n",rs) ;
#endif

	    } /* end for */
	} /* end if (arguments) */
#endif /* COMMENT */

	if ((ifname == NULL) || (ifname[0] == '\0') || (ifname[0] == '-'))
		ifname = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) >= 0) {
	    FINDINLINE	fi ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    const char	*fmt ;
	    char	lbuf[LINEBUFLEN+1] ;
	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
		len = rs ;

		bprintln(ofp,lbuf,len) ;

		if ((sl = findinline(&fi,lbuf,len)) > 0) {
		    fmt = "» sl=%u k=%t v=>%t<\n" ;
		    bprintf(ofp,fmt,sl,fi.kp,fi.kl,fi.vp,fi.vl) ;
		}

#if	CF_DEBUGS
	        debugprintf("main: findinline() rs=%d\n",sl) ;
#endif

	    } /* end while */
	    bclose(ifp) ;
	} /* end if (infile-open) */

	    bclose(ofp) ;
	} /* end if (outfile-open) */

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


