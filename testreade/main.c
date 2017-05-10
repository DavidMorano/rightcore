/* main (testreade) */


#define	CF_DEBUGS	1		/* compile-time debugging */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	int	rs = SR_OK ;
	int	fd_in = FD_STDIN ;
	int	fd_out = FD_STDOUT ;
	int	len ;
	int	to = 3 ;
	int	opts = FM_TIMED ;
	int	ex = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	while (rs >= 0) {

	    rs = uc_reade(fd_in,linebuf,LINEBUFLEN,to,opts) ;
	    len = rs ;

#if	CF_DEBUGS
	debugprintf("main: uc_reade() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
		break ;

	    if (len == 0)
		break ;

	    rs = u_write(fd_out,linebuf,len) ;
	    if (rs < 0)
		break ;

	} /* end while */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


