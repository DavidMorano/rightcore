/* main (testbfile) */


#define	CF_DEBUG	0
#define	CF_WRITE	1
#define	CF_ERROUT	0


/* revision history:

	= February 88, David A­D­ Morano

	This is something from way back !  (near the beginning maybe)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This is a test program for the BFILE package.


*************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define	VERSION		"1"
#define	NPARG		2
#define	BUFLEN		200
#define	OUTPUT		"/home/dam/src/rmailerd/testput.out"



/* external subroutines */


/* external variables */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	int	len ;
	int	pan, i ;
	int	argl, aol ;
	int	rs ;
	int	lines ;
	int	err_fd ;
	int	ex = 0 ;
	int	f_usage = FALSE ;

	char	*progname, *argp, *aop ;
	char	buf[BUFLEN + 1] ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUG
		debugprintf("main: about to open error\n") ;
#endif

	progname = argv[0] ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;

#if	CF_DEBUG
		debugprintf("main: about to open input\n") ;
#endif

	rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0) {

#if	CF_DEBUG
		debugprintf("main: no open input, rs=%d\n",rs) ;
#endif

		ex = NOINPUT ;
		goto done ;
	}


#if	CF_DEBUG
		debugprintf("main: about to open output\n") ;
#endif

	if ((rs = bopen(ofp,OUTPUT,"wca",0666)) < 0) {

#if	CF_DEBUG
		debugprintf("main: no open output, rs=%d\n",rs) ;
#endif

		ex = EX_CANTCREAT ;
		goto done ;
	}


/* OK, go into infinite loop mode */

#if	CF_DEBUG
		debugprintf("main: before loop\n",rs) ;
#endif

	lines = 0 ;
	while (TRUE) {

#if	CF_DEBUG
		debugprintf("main: before read\n",rs) ;
#endif

	if ((len = breadline(ifp,buf,BUFLEN)) <= 0) 
		break ;

		lines += 1 ;

#if	CF_DEBUG
		debugprintf("main: before write\n",rs) ;
#endif

#if	CF_WRITE
		if ((rs = bwrite(ofp,buf,len)) < 0)
			break ;
#endif /* CF_WRITE */

#if	CF_DEBUG
		debugprintf("main: bottom loop \n",rs) ;
#endif

	} /* end while */

#if	CF_DEBUG
		debugprintf("main: after loop\n") ;
#endif

	bclose(ifp) ;

	bclose(ofp) ;


	if (len < 0) {

#if	CF_DEBUG
		debugprintf("main: bad read, len=%d\n",len) ;
#endif

		goto baddone ;
	}

	if (rs < 0) {

#if	CF_DEBUG
		debugprintf("main: bad write, rs=%d\n",rs) ;
#endif

		goto baddone ;
	}


#if	CF_ERROUT
	bprintf(efp,"%s: lines=%d\n",progname,lines) ;
#endif


done:


baddone:
	bclose(efp) ;

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
	bprintf(efp,"%s: not enough arguments supplied\n",
	    progname) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument value specified\n",
	    progname) ;

badret:
	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



