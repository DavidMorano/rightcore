/* main (testbfile) */

/* program to test the 'bfile(3b)' subroutine library */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_READ		0		/* try reading */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written to do some testing
	on the BFILE package.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a test program for the 'uc_open(3uc)' subroutine.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"bfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */

static int procfile(struct proginfo *,bfile *,const char *) ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs = SR_OK ;
	int	pan = 0 ;
	int	argl, aol ;
	int	ex = EX_INFO ;
	int	f_usage = FALSE ;

	const char	*progname, *argp, *aop ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	progname = argv[0] ;

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;

	if (bopen(efp,efname,"wca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	if (ofname == NULL) ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

#if	CF_READ
	{
	    int		ai = 1 ;
	    const char	*fname ;

	    if (argc > 1) {
		for (ai = 1 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
		    fname = argv[ai] ;

		    pan += 1 ;
		    rs = procfile(pip,ofp,fname) ;

		    if (rs < 0) break ;
		} /* end while */
	    } /* end if (arguments) */

	    if ((rs >= 0) && (pan == 0)) {
		    fname = STDINFNAME ;
		    rs = procfile(pip,ofp,fname) ;
	    }

	}
#else
	    rs = bprintf(ofp,"hello world!\n") ;
#endif /* CF_READ */

	    bclose(ofp) ;
	} /* end if (file-open) */

done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

retearly:
	bclose(efp) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int procfile(struct proginfo *pip,bfile *ofp,const char *fname)
{
	bfile	ifile, *ifp = &ifile ;
	int	rs ;
	int	wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("procfile: fname=%s\n",fname) ;
#endif

	if ((fname == NULL) || (fname[0] == '\0') || (fname[0] == '-'))
	    fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    const int	to = -1 ;
	    const int	ro = 0 ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN+1] ;

	    while ((rs = bread(ifp,lbuf,llen)) > 0) {
		len = rs ;

#if	CF_DEBUGS
		debugprintf("procfile: l=>%t<\n",
		    lbuf,strlinelen(lbuf,len,50)) ;
#endif

		rs = bwrite(ofp,lbuf,len) ;
		wlen += rs ;

		if (rs < 0) break ;
	    } /* end while */

	    bclose(ifp) ;
	} /* end if (bfile) */

#if	CF_DEBUGS
	debugprintf("procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


