/* main (testput) */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_WRITE	1
#define	CF_ERROUT	0


/* revision history :

	= 1988-02-01, David A­D­ Morano
	This is something from way back !  (near the beginning maybe)

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This is a test program for the BIO package.


*************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<localmisc.h>


/* local defines */

#define	VERSION		"1"
#define	NPARG		2
#define	BUFLEN		200
#define	OUTPUT		"/home/dam/src/rmailerd/testput.out"


/* external subroutines */


/* external variables */


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	int	rs ;
	int	len ;
	int	pan, i ;
	int	argl, aol ;
	int	lines ;
	int	err_fd ;
	int	f_usage = FALSE ;

	cchar	*progname, *argp, *aop ;
	cchar	*cp ;
	char	buf[BUFLEN + 1] ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    esetfd(err_fd) ;


#if	CF_DEBUG
		eprintf("main: about to open error\n") ;
#endif

	progname = argv[0] ;
	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


#if	CF_DEBUG
		eprintf("main: about to open input\n") ;
#endif

	if ((rs = bopen(ifp,BIO_STDIN,"dr",0666)) < 0) {

#if	CF_DEBUG
		eprintf("main: no open input, rs=%d\n",rs) ;
#endif

		return rs ;
	}


#if	CF_DEBUG
		eprintf("main: about to open output\n") ;
#endif

	if ((rs = bopen(ofp,OUTPUT,"wca",0666)) < 0) {

#if	CF_DEBUG
		eprintf("main: no open output, rs=%d\n",rs) ;
#endif

		return rs ;
	}


/* OK, go into infinite loop mode */

#if	CF_DEBUG
		eprintf("main: before loop\n",rs) ;
#endif

	lines = 0 ;
	while (TRUE) {

#if	CF_DEBUG
		eprintf("main: before read\n",rs) ;
#endif

	if ((len = breadline(ifp,buf,BUFLEN)) <= 0) break ;

		lines += 1 ;

#if	CF_DEBUG
		eprintf("main: before write\n",rs) ;
#endif

#if	CF_WRITE
		if ((rs = bwrite(ofp,buf,len)) < 0)
			break ;
#endif /* CF_WRITE */

#if	CF_DEBUG
		eprintf("main: bottom loop \n",rs) ;
#endif

	} /* end while */

#if	CF_DEBUG
	eprintf("main: after loop\n") ;
#endif

	bclose(ifp) ;

	bclose(ofp) ;


	if (len < 0) {

#if	CF_DEBUG
		eprintf("main: bad read, len=%d\n",len) ;
#endif

		goto baddone ;
	}

	if (rs < 0) {

#if	CF_DEBUG
		eprintf("main: bad write, rs=%d\n",rs) ;
#endif

		goto baddone ;
	}


#if	CF_ERROUT
	bprintf(efp,"%s: lines=%d\n",progname,lines) ;
#endif


done:


baddone:
	bclose(efp) ;

	return OK ;

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



