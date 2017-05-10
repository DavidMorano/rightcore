/* maintermout (C99) */


#define	CF_DEBUGS	1		/* compile-time debugging */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"termout.h"

#include	"config.h"
#include	"defs.h"


#ifndef	COLUMNS
#define	COLUMNS	80
#endif

/* external subroutines */

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int main(int argc,const char **argv,const char **envv) {
	TERMOUT		outer, *top = &outer ;

	const int	ncols = COLUMNS ;

	int	rs ;

	const char	*termtype = "vt520" ;
	const char	*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif

	if ((rs = termout_start(top,termtype,-1,ncols)) >= 0) {
	    const char	*sp = " a\ba :\bb *\bc _\bd `\be __\b\bff _\b:\bg " ;

#if	CF_DEBUGS
	debugprintf("main: termout_load()\n") ;
#endif

	    if ((rs = termout_load(top,sp,-1)) >= 0) {
		const char	*lp ;
		int	i ;
		int	ll ;

#if	CF_DEBUGS
	debugprintf("main: termout_getline()\n") ;
#endif

		for (i = 0 ; (ll = termout_getline(top,i,&lp)) >= 0 ; i += 1) {

#if	CF_DEBUGS
		debugprintf("main: termout_getline() ll=%d\n",ll) ;
		debugprintf("main: l=>%t<\n",lp,ll) ;
#endif

	    	    printf("l=>%s<\n",lp) ;
		} /* end for */

#if	CF_DEBUGS
		debugprintf("main: for-after\n") ;
#endif

	    } /* end if */

	    termout_finish(top) ;
	} /* end if (termout) */

#if	CF_DEBUGS
	debugprintf("main: exiting \n") ;
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


