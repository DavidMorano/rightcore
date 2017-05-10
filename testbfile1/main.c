/* main (testbfile) */


#define	CF_DEBUGS	1
#define	CF_WRITE	1
#define	CF_LITTLE	1		/* do the little output */
#define	CF_ERRSUM	0		/* error summary */
#define	CF_READLINE	1		/* breadline(3bfile) ? */


/* revision history:

	= 88/02/01, David A­D­ Morano

	This subroutine was originally written to do some testing
	on the BIO package.


*/


/************************************************************************

	This is a test program for the BFILE package.


*************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<time.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"bfile.h"



/* local defines */

#define	NPARG		2

#ifndef	BUFLEN
#define	BUFLEN		200
#endif



/* external subroutines */


/* external variables */







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	int	rs, i ;
	int	len ;
	int	lines ;
	int	ex = EX_USAGE ;
	int	f_usage = FALSE ;

	char	*progname, *argp, *aop ;
	char	buf[BUFLEN + 1], *bp ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("main: about to open error\n") ;
#endif

	progname = argv[0] ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


#if	CF_DEBUGS
	debugprintf("main: about to open input\n") ;
#endif

	if ((argc >= 2) && (argv[1][0] != '\0'))
	    rs = bopen(ifp,argv[1],"r",0666) ;

	else
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("main: bopen() rs=%d\n",rs) ;
#endif

	    ex = EX_NOINPUT ;
	    goto done ;
	}


#if	CF_DEBUGS
	debugprintf("main: about to open output\n") ;
#endif

	if ((argc >= 3) && (argv[2][0] != '\0'))
	    rs = bopen(ofp,argv[2],"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("main: no open output, rs=%d\n",rs) ;
#endif

	    ex = EX_CANTCREAT ;
	    goto done ;
	}


/* OK, go into infinite loop mode */

#if	CF_DEBUGS
	debugprintf("main: before loop\n",rs) ;
#endif

	lines = 0 ;
	while (rs >= 0) {

#if	CF_READLINE
	    rs = breadline(ifp,buf,BUFLEN) ;
#else
	    rs = bread(ifp,buf,BUFLEN) ;
#endif

#if	CF_DEBUGS
	    debugprintf("main: breadline() rs=%d\n",rs) ;
#endif

		len = rs ;
	    if (rs < 0)
	        break ;

	    if (len == 0)
		break ;

	    lines += 1 ;

#if	CF_WRITE
	    rs = bwrite(ofp,buf,len) ;

#if	CF_DEBUGS
	    debugprintf("main: bwrite() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	        break ;
#endif /* CF_WRITE */

#if	CF_DEBUGS
	    debugprintf("main: bottom loop \n",rs) ;
#endif

	} /* end while */

#if	CF_DEBUGS
	debugprintf("main: after loop\n") ;
#endif

	bclose(ofp) ;

	bclose(ifp) ;


	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("main: bad read or write rs=%d\n",rs) ;
#endif

	    goto baddone ;
	}


#if	CF_ERRSUM
	bprintf(efp,"%s: lines=%d\n",progname,lines) ;
#endif


done:
ret2:
	ex = EX_OK ;

ret1:
	bclose(efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

usage:
	bprintf(efp,
	    "usage: %s [mailfile [offset]] [-sV] [-t offset] [-offset]\n",
	    progname) ;

	goto badret ;

not_enough:
	ex = EX_USAGE ;
	bprintf(efp,"%s: not enough arguments supplied\n",
	    progname) ;

	goto badret ;

badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad argument value specified\n",
	    progname) ;

baddone:
badret:
	ex = EX_USAGE ;
	goto ret1 ;
}
/* end subroutine (main) */



