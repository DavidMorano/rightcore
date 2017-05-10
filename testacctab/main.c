/* main */

/* test out the ACCTAB object in acid trials */


#define	CF_DEBUGS	1



#include	<sys/types.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<randomvar.h>

#include	"acctab.h"

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */






int main()
{
	bfile	outfile ;

	RANDOMVAR	rv ;

	ACCTAB		st ;

	void	*ep, *endp ;

	uint	uiw ;

	int	rs, i ;
	int	j, k, n ;
	int	fd_debug = -1 ;

	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	bopen(&outfile,BFILE_STDOUT,"wct",0666) ;


#if	CF_DEBUGS
	debugprintf("main: acctab_open()\n") ;
#endif

	rs = acctab_open(&st,"acctab",NULL) ;

#if	CF_DEBUGS
	debugprintf("main: acctab_open() rs=%d\n",rs) ;
#endif


	for (i = 0 ; i < 1000 ; i += 1) {

	    for (j = 0 ; j < 100 ; j += 1) {

#if	CF_DEBUGS
	debugprintf("main: acctab_check()\n") ;
#endif

	        rs = acctab_check(&st,NULL) ;

	        if (rs < 0)
	            bprintf(&outfile,"acctab_check() rs=%d\n",rs) ;

	        sleep(1) ;

	    } /* end for */


#if	CF_DEBUGS
	debugprintf("main: u_sbrk()\n") ;
#endif

	    u_sbrk(0,&endp) ;

	    bprintf(&outfile,"endp=%08lx\n",endp) ;

	    bflush(&outfile) ;

	} /* end for */


#if	CF_DEBUGS
	debugprintf("main: acctab_close()\n") ;
#endif

	acctab_close(&st) ;

	u_sbrk(0,&endp) ;

	bprintf(&outfile,"final endp=%08lx\n",endp) ;



	bclose(&outfile) ;

	return 0 ;
}



