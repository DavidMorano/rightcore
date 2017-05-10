/* testmsu */


#define	CF_DEBUGS	0		/* compile-time */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	"msuclients.h"


#define	VARDEBUGFNAME	"TESTMSU_DEBUGFILE"
#define	VARDEBUGFD1	"TESTMSU_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#define	PRLOCAL		"/usr/add-on/local"
#define	TO_MSU		10

/* exported subroutines */

int main()
{
	MSUCLIENTS	mc ;
	FILE		*ofp = stdout ;
	int		rs ;
	int		to = TO_MSU ;
	const char	*pr ;
	cchar		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if ((pr = getenv(VARPRLOCAL)) == NULL)
		pr = PRLOCAL ;

/* object, program-root, req-filename, time-out */

	if ((rs = msuclients_open(&mc,pr,NULL,to)) >= 0) {
	    fprintf(ofp,"main: msuclients_init() rs=%d\n",rs) ;
	    rs = msuclients_status(&mc) ;
	    fprintf(ofp,"main: msuclients_status() rs=%d\n",rs) ;
	    msuclients_close(&mc) ;
	} /* end if (msuclients) */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


