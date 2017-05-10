/* testsysrealname */

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

#include	"sysrealname.h"

#define	VARDEBUGFNAME	"TESTSYSREALNAME_DEBUGFILE"

#define	NDEBFNAME	"testsysrealname.deb"

/* external subroutines */

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

/* local structures */

struct arginfo {
	int		argc ;
	int		ai, ai_max, ai_pos ;
	const char	**argv ;
} ;

/* forward references */

static int procargs(struct ainfo *,SYSREALNAME *) ;

/* exported subroutines */

int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct arginfo	ainfo ;
	SYSREALNAME	grm ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;
	int	ai, ai_pos, ai_max ;
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

	ai = 0 ;
	ai_pos = 0 ;
	ai_max = 0 ;

	memset(&ainfo,0,sizeof(struct arginfo)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;


	if ((rs = sysrealname_start(&grm,max,ttl)) >= 0) {
	    if (argv != NULL) {
		rs = procargs(&ainfo,&grm) ;
	    } /* end if (arguments) */
	    rs1 = sysrealname_finish(&grm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sysrealname) */

#if	CF_DEBUGS
	debugprintf("main: sysrealname-out rs=%d\n",rs) ;
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


/* local subroutines */


static int procargs(struct ainfo *aip,SYSREALNAME *snp)
{
	int	rs = SR_OK ;
	int	ai ;
		int	c = 0 ;

		for (ai = 1 ; argv[ai] != NULL ; ai += 1) {
		    SYSREALNAME_CUR	cur ;
		    const char	*gn = argv[ai] ;
#if	CF_PRINTBEF
			      printf("before cursor gn=%s\n",gn) ;
#endif
		    if ((rs = sysrealname_curbegin(&grm,&cur)) >= 0) {
#if	CF_PRINTBEF
			      printf("before lookup gn=%s\n",gn) ;
#endif
			if ((rs1 = sysrealname_lookup(&grm,&cur,gn,-1)) >= 0) {
			  char		ub[USERNAMELEN+1] ;
			  const int	ul = USERNAMELEN ;
#if	CF_PRINTBEF
			      printf("before lookread gn=%s\n",gn) ;
#endif
			  while ((rs = sysrealname_lookread(&grm,&cur,ub,ul)) >= 0) {
				c += 1 ;
#if	CF_DEBUGS
		debugprintf("main: sysrealname_lookread() rs=%d\n",rs) ;
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
			sysrealname_curend(&grm,&cur) ;
		    } /* end if (cursor) */
		} /* end for */

	return rs ;
}
/* end subroutine (procargs) */


