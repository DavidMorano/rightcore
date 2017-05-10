/* testgrcache */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<stdio.h>
#include	<getbufsize.h>
#include	<vsystem.h>
#include	<grcache.h>

#define	VARDEBUGFNAME	"TESTGRCACHE_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	GRCACHE		g ;
#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif
	int		rs ;
	int		rs1 ;

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

	if ((rs = grcache_start(&g,4,4)) >= 0) {
	    int			n = 0 ;
#if	CF_DEBUGS
	    debugprintf("main: grcache_start() rs=%d\n",rs) ;
#endif

	    if (argv != NULL) {
	        struct group	gr ;
	        const int	grlen = getbufsize(getbufsize_gr) ;
	        char		*grbuf ;
		if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	            int		ai ;
		    cchar	**av = argv ;
	            const char	*gn ;
	            for (ai = 1 ; (ai < argc) && (av[ai] != NULL) ; ai += 1) {
	                n += 1 ;
	                gn = argv[ai] ;
	                rs = grcache_lookname(&g,&gr,grbuf,grlen,gn) ;
#if	CF_DEBUGS
	                debugprintf("main: grcache_lookname() rs=%d\n",rs) ;
#endif
		        if (rs >= 0) {
	                    printf("gn=%s gid=%d\n",gn,gr.gr_gid) ;
		        } else if (rs == SR_NOTFOUND) {
			    rs = SR_OK ;
	                    printf("gn=%s not-found\n",gn) ;
		        }
	                if (rs < 0) break ;
	            } /* end for */
		    uc_free(grbuf) ;
		} /* end if (memory-allocation) */
	    } /* end if (non-null) */

#if	CF_DEBUGS
	    debugprintf("main: out rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && (n > 0)) {
	        GRCACHE_STATS	s ;
	        if ((rs = grcache_stats(&g,&s)) >= 0) {
	            printf("stats nent=%u\n",s.nentries) ;
	            printf("stats acc=%u\n",s.total) ;
	            printf("stats ref=%u\n",s.refreshes) ;
	            printf("stats phit=%u\n",s.phits) ;
	            printf("stats pmis=%u\n",s.pmisses) ;
	            printf("stats nhit=%u\n",s.nhits) ;
	            printf("stats nmis=%u\n",s.nmisses) ;
	        }
#if	CF_DEBUGS
	        debugprintf("main: ugetpw_stats() rs=%d\n",rs) ;
#endif
	    } /* end if */

	    rs1 = grcache_finish(&g) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (grcache) */

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


