/* main (testishex) */
/* lang=C99 */

#define	CF_DEBUGS	1

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test the |ishexlatin| subroutine.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<stdio.h>
#include	<vecpstr.h>
#include	<cthex.h>
#include	<vsystem.h>

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40
#endif

#define	VARDEBUGFNAME	"TESTISHEX_DEBUGFILE"

extern int	ishexlatin(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar 	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;

int main(int argc,const char **argv,const char **envv)
{
	vecpstr		nsl ;
	const int	n = (10*1024*1024) ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if ((rs = vecpstr_start(&nsl,n,0,0)) >= 0) {

/* phase 1 */

	    if (rs >= 0) {
	        const int	dlen = DIGBUFLEN ;
	        int		i ;
	        int		dl ;
	        char		dbuf[DIGBUFLEN+1] ;
	        for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	            if ((dl = cthex(dbuf,dlen,i)) >= 0) {
	                rs = vecpstr_add(&nsl,dbuf,dl) ;
	            }
	        } /* end for */
	    } /* end if (ok) */

/* phase 2 */

#if	CF_DEBUGS
	    debugprintf("main: p2\n") ;
#endif

	    if (rs >= 0) {
	        hrtime_t	t0, t1 ;
	        int		i ;
	        int		f = FALSE ;
	        cchar		**va ;
	        if ((vecpstr_getvec(&nsl,&va)) >= 0) {
	            cchar	*cp ;
	            t0 = gethrtime() ;
	            for (i = 0 ; i < n ; i += 1) {
	                int	j ;
	                cp = va[i] ;
	                for (j = 0 ; cp[j] ; j += 1) {
	                    int	ch = MKCHAR(cp[j]) ;
	                    f = ishexlatin(ch) ;
	                    if (! f) break ;
	                }
	                if (! f) break ;
	            } /* end for */
	            t1 = gethrtime() ;
#if	CF_DEBUGS
	            debugprintf("main: print-out\n") ;
#endif
	            fprintf(stdout,"f=%u td=%llu\n",f,(t1-t0)) ;
	        } /* end if (vecpstr_getvec) */
	    } /* end if (ok) */

/* done */

	    rs1 = vecpstr_finish(&nsl) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecpstr) */

#if	CF_DEBUGS
	debugprintf("main: done rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


