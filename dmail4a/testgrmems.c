/* testgrmems */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_PRINTBEF	0		/* use |printf(3stdio)| before */
#define	CF_PRINTOUT	1		/* use |printf(3stdio)| for out */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/syslog.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<ucmallreg.h>
#include	<localmisc.h>

#include	"grmems.h"

#define	VARDEBUGFNAME	"TESTGRMEMS_DEBUGFILE"

#define	NDEBFNAME	"testgrmems.deb"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	GRMEMS	grm ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs ;
	int	rs1 ;
	int	max = 10 ;
	int	ttl = (10*60) ;
	int	opts = 0 ;

	const char	*cp ;

#if	CF_DEBUGS
	if ((cp = getenv(VARDEBUGFNAME)) != NULL) {
	    debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((rs = grmems_start(&grm,max,ttl)) >= 0) {
	    if (argv != NULL) {
		int	ai ;
		int	c = 0 ;
		for (ai = 1 ; argv[ai] != NULL ; ai += 1) {
		    GRMEMS_CUR	cur ;
		    const char	*gn = argv[ai] ;
#if	CF_PRINTBEF
			      printf("before cursor gn=%s\n",gn) ;
#endif
		    if ((rs = grmems_curbegin(&grm,&cur)) >= 0) {
#if	CF_PRINTBEF
			      printf("before lookup gn=%s\n",gn) ;
#endif
			if ((rs1 = grmems_lookup(&grm,&cur,gn,-1)) >= 0) {
			  char		ub[USERNAMELEN+1] ;
			  const int	ul = USERNAMELEN ;
#if	CF_PRINTBEF
			      printf("before lookread gn=%s\n",gn) ;
#endif
			  while ((rs = grmems_lookread(&grm,&cur,ub,ul)) >= 0) {
				c += 1 ;
#if	CF_DEBUGS
		debugprintf("main: grmems_lookread() rs=%d\n",rs) ;
		debugprintf("main: ub=%s\n",ub) ;
#endif
#if	CF_PRINTOUT
			      printf("%s\n",ub) ;
#endif
			  } /* end while */
			  if (rs == SR_NOTFOUND) {
				rs = SR_OK ;
#if	CF_DEBUGS
		debugprintf("main: while-out done c=%u\n",c) ;
#endif
			   }
			} else {
			    printf("not-found (%d)\n",rs1) ;
			} /* end if */
			grmems_curend(&grm,&cur) ;
		    } /* end if (cursor) */
		} /* end for */
	    } /* end if (arguments) */
	    rs1 = grmems_finish(&grm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (grmems) */

#if	CF_DEBUGS
	debugprintf("main: grmems-out rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    const int	f_default = FALSE ;
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if ((mdiff > 0) || f_default) {
		UCMALLREG_CUR	cur ;
		UCMALLREG_REG	reg ;
		const int	size = (10*sizeof(uint)) ;
		int		rs1 ;
		const char	*ids = "main" ;
		uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
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


