/* main (testbfile) */
/* lang=C89 */

#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory-allocations */
#define	CF_WRITE	0
#define	CF_LITTLE	1		/* do the little output */
#define	CF_ERRSUM	0		/* error summary */


/* revision history:

	= 1988-02-01, David A­D­ Morano
        This subroutine was originally written to do some testing on the BIO
        package.

	= 2015-02-22, David A­D­ Morano
	I'm thinking about making some changes to this test.

*/

/* Copyright © 1988,2015 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a test program for the BFILE package.

	Synopsis for running:

	$ testbfile.x <file> > outfile


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	1024
#endif

#ifndef	UEBUFLEN
#define	UEBUFLEN	UTMPACCENT_BUFLEN
#endif

#ifndef FILEBUF_RCNET
#define	FILEBUF_RCNET	4		/* read-count for network */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	VARDEBUGFNAME	"TESTBFILE_DEBUGFILE"


/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char 	*getourenv(const char **,const char *) ;

extern char	*timestr_logz(time_t,char *) ;


/* forward references */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile	ofile, *ofp = &ofile ;
	bfile	ifile, *ifp = &ifile ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	pan = 0 ;
	int	i ;
	int	argl, aol ;
	int	ex = EX_USAGE ;
	int	len ;
	int	lines ;
	int	f_usage = FALSE ;

	const char	*progname ;
	const char	*argp, *aop ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*cp ;

	char		lbuf[LINEBUFLEN + 1] ;


#if	CF_DEBUGS
	{
	    const char	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL)
	        debugopen(cp) ;
	    debugprintf("main: starting\n") ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

#if	CF_DEBUGS
	debugprintf("main: ent\n") ;
#endif

	progname = argv[0] ;

	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: about to open output\n") ;
#endif

	    if ((argc >= 2) && (argv[1][0] != '\0')) {
	        ifname = argv[1] ;
	    } else
	        ifname = BFILE_STDIN ;

	    if ((rs = bopen(ifp,ifname,"r",0666)) >= 0) {
		offset_t	loff = 0 ;
	        int		ll ;
		const char	*lp = lbuf ;

	        while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	            len = rs ;
	            lines += 1 ;

		    ll = len ;
		    if (ll && (lbuf[ll-1] == '\n')) ll -= 1 ;

#if	CF_DEBUGS
	            debugprintf("main: l=>%t<\n",
			lp,strlinelen(lp,ll,50)) ;
	            debugprintf("main: before write\n") ;
#endif

	            rs = bwrite(ofp,lbuf,len) ;

#if	CF_DEBUGS
	            debugprintf("main: bwrite() rs=%d\n",rs) ;
#endif

		    loff += len ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

#if	CF_DEBUGS
	        debugprintf("main: while-out rs=%d\n",rs) ;
#endif

	        bclose(ifp) ;
	    } /* end if (ifile) */

	    bclose(ofp) ;
	} /* end if (ofile) */

	ex = (rs >= 0) ? EX_OK : EX_SOFTWARE ;

#if	CF_DEBUGS
	debugprintf("main: all-out rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


