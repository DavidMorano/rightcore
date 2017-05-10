/* main (testput) */


#define	F_DEBUG		0
#define	F_WRITE		1
#define	F_ERROUT	0


/* revision history :

	= February 88, David A­D­ Morano

	This is something from way back !  (near the beginning maybe)


*/



/************************************************************************

	This is a test program for the BIO package.



*************************************************************************/




#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<bfile.h>

#include	"misc.h"




/* local defines */

#define	VERSION		"1"
#define	NPARG		2
#define	BUFLEN		200
#define	OUTPUT		"/home/dam/src/rmailerd/testput.out"



/* external subroutines */


/* external variables */





int main(argc,argv)
int	argc ;
char	*argv[] ;
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
	int	f_usage = FALSE ;

	char	*progname, *argp, *aop ;
	char	buf[BUFLEN + 1] ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    esetfd(err_fd) ;


#if	F_DEBUG
		eprintf("main: about to open error\n") ;
#endif

	progname = argv[0] ;
	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


#if	F_DEBUG
		eprintf("main: about to open input\n") ;
#endif

	if ((rs = bopen(ifp,BIO_STDIN,"dr",0666)) < 0) {

#if	F_DEBUG
		eprintf("main: no open input, rs=%d\n",rs) ;
#endif

		return rs ;
	}


#if	F_DEBUG
		eprintf("main: about to open output\n") ;
#endif

	if ((rs = bopen(ofp,OUTPUT,"wca",0666)) < 0) {

#if	F_DEBUG
		eprintf("main: no open output, rs=%d\n",rs) ;
#endif

		return rs ;
	}


/* OK, go into infinite loop mode */

#if	F_DEBUG
		eprintf("main: before loop\n",rs) ;
#endif

	lines = 0 ;
	while (TRUE) {

#if	F_DEBUG
		eprintf("main: before read\n",rs) ;
#endif

	if ((len = bgetline(ifp,buf,BUFLEN)) <= 0) break ;

		lines += 1 ;

#if	F_DEBUG
		eprintf("main: before write\n",rs) ;
#endif

#if	F_WRITE
		if ((rs = bwrite(ofp,buf,len)) < 0)
			break ;
#endif /* F_WRITE */

#if	F_DEBUG
		eprintf("main: bottom loop \n",rs) ;
#endif

	} /* end while */

#if	F_DEBUG
		eprintf("main: after loop\n") ;
#endif

	bclose(ifp) ;

	bclose(ofp) ;


	if (len < 0) {

#if	F_DEBUG
		eprintf("main: bad read, len=%d\n",len) ;
#endif

		goto baddone ;
	}

	if (rs < 0) {

#if	F_DEBUG
		eprintf("main: bad write, rs=%d\n",rs) ;
#endif

		goto baddone ;
	}


#if	F_ERROUT
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



