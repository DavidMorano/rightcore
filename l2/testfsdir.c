/* testfsdir2 (C89) */
#define	CF_DEBUGS	1
#include	<envstandards.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	"fsdir2.h"

#define	VARDEBUGFNAME	"TESTFSDIR_DEBUGFILE"

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

int main(int argc,const char **argv,const char **envv)
{
	fsdir2		d ;
	fsdir2_ent	ds ;
	const int	n = 10 ;
	int	rs ;
	int	i ;

#if	CF_DEBUGS
	{
	    const char	*cp ;
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */


	if ((rs = fsdir2_open(&d,"jj")) >= 0) {
	    for (i = 0 ; i < n ; i += 1) {
	    rs = fsdir2_read(&d,&ds) ;
	    printf("rs=%d reclen=%u name=%s\n",rs,ds.reclen,ds.name) ;
		if (rs <= 0) break ;
	    }
	    fsdir2_close(&d) ;
	}

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */

