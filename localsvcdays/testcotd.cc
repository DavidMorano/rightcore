/* testcotd */
/* lang=C++98 */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<localmisc.h>
#include	"testcotd.h"

#ifndef	FD_STDOUT
#define	FD_STDOUT	1
#endif

/* external subroutines */

extern "C" int	main(int,const char **,const char **) ;

#if	CF_DEBUGS
extern "C" int	debugopen(const char *) ;
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" const char	*getourenv(const char **,const char *) ;


int main(int argc,const char **argv,const char **envv)
{
	const int	ofd = FD_STDOUT ;
	int		rs ;
	const char	*fn = "local§cotd" ;
	const char	*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if ((argc > 1) && (argv[1] != '\0')) fn = argv[1] ;

	if ((rs = uc_open(fn,O_RDONLY,0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		fd = rs ;
		char		lbuf[LINEBUFLEN+1] ;
		while ((rs = u_read(fd,lbuf,llen)) > 0) {
		    int	ll = rs ;
		    rs = u_write(ofd,lbuf,ll) ;
		    if (rs < 0) break ;
		} /* end while */
		u_close(fd) ;
	} /* end if (open) */

	fprintf(stderr,"main: ret rs=%d\n",rs) ;

#if	CF_DEBUGS
	debugprintf("main: exiting rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


