/* testpcsunodes */
/* lang=C89 */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<vsystem.h>
#include	<bfile.h>
#include	<ucmallreg.h>
#include	<localmisc.h>
#include	"pcsunodes.h"

#define	VARDEBUGFNAME	"TESTPCSUNODES_DEBUGFILE"

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#ifdef	REALNAMELEN
#define	RBUFLEN		REALNAMELEN
#else
#define	RBUFLEN		100
#endif

extern int	sninetaddr(char *,int,int,const char *) ;
extern int	cfdeci(cchar *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	bfile		ofile, *ofp = &ofile ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*ofn = BFILE_STDOUT ;
	cchar		*pr = getourenv(envv,VARPRPCS) ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    PCSUNODES	un ;
	    if ((rs = pcsunodes_start(&un,pr)) >= 0) {
		int	i ;
		for (i = 0 ; pcsunodes_get(&un,i,&cp) >= 0 ; i += 1) {
		    rs = bprintln(ofp,cp,-1) ;
		    if (rs < 0) break ;
		} /* end for */
		rs1 = pcsunodes_finish(&un) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (pcsunodes) */
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        cchar		*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",
	            mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        }
	        ucmallreg_curend(&cur) ;
	        debugprintf("main: MIdone\n") ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


