/* testugetpw */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<getbufsize.h>
#include	<ugetpw.h>

#define	VARDEBUGFNAME	"TESTUGETPW_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;


#if	CF_DEBUGS
	{
	    cchar	*cp ;
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
	    if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	        struct passwd	pw ;
	        const int	pwlen = rs ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            int	ai ;
	            for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	                cchar	*un = argv[ai] ;
#if	CF_DEBUGS
	                debugprintf("main: u=%s\n",un) ;
#endif
	                if ((rs1 = ugetpw_name(&pw,pwbuf,pwlen,un)) >= 0) {
			    const uid_t	uid = pw.pw_uid ;
			    cchar	*dir = pw.pw_dir ;
	                    printf("u=%s uid=%d home=%s\n",un,uid,dir) ;
	                } else if (rs1 == SR_NOTFOUND) {
	                    rs = SR_OK ;
	                    printf("u=%s not_found (%d)\n",un,rs1) ;
	                }
#if	CF_DEBUGS
	                debugprintf("main: ugetpw_name() rs=%d\n",rs1) ;
#endif
	                if (rs < 0) break ;
	            } /* end for */
		    rs1 = uc_free(pwbuf) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (memory-allocation) */
	    } /* end if (getbufsize) */
	} /* end if (ugetpw) */

#if	CF_DEBUGS
	debugprintf("main: out rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    struct ugetpw_stats	s ;
	    if ((rs = ugetpw_stats(&s)) >= 0) {
	        printf("stats max=%u\n",s.max) ;
	        printf("stats ttl=%u\n",s.ttl) ;
	        printf("stats nent=%u\n",s.nent) ;
	        printf("stats acc=%u\n",s.acc) ;
	        printf("stats phit=%u\n",s.phit) ;
	        printf("stats pmis=%u\n",s.pmis) ;
	        printf("stats nhit=%u\n",s.nhit) ;
	        printf("stats nmis=%u\n",s.nmis) ;
	    }
#if	CF_DEBUGS
	    debugprintf("main: ugetpw_stats() rs=%d\n",rs) ;
#endif
	} /* end if */

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


