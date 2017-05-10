/* main (testsend) */


#define	CF_DEBUG	0
#define	F_WRITE		1
#define	F_ERROUT	1


/* revision history:

	= David A.D. Morano, February 88


*/


/************************************************************************

	This is a test program for sending a file someplace.


*************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<pwd.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define	VERSION		"1"
#define	NPARG		2
#define	BUFLEN		200
#define	OUTPUT		"/home/dam/src/rmailerd/testsend.out"



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


	if (((cp = getenv("DEBUGFD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


#if	CF_DEBUG
		debugprintf("main: about to open error\n") ;
#endif

	progname = argv[0] ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


#if	CF_DEBUG
		debugprintf("main: about to open input\n") ;
#endif

	if ((rs = bopen(ifp,BFILE_STDIN,"dr",0666)) < 0) {

#if	CF_DEBUG
		debugprintf("main: no open input, rs=%d\n",rs) ;
#endif

		return rs ;
	}


#if	CF_DEBUG
		debugprintf("main: about to open output\n") ;
#endif

	if ((rs = bopen(ofp,OUTPUT,"wca",0666)) < 0) {

#if	CF_DEBUG
		debugprintf("main: no open output, rs=%d\n",rs) ;
#endif

		return rs ;
	}


/* pop out the arguments */

	for (i = 0 ; i < argc ; i += 1)
		bprintf(ofp,"argv[%d]=%s\n",i,argv[i]) ;


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

#if	F_WRITE
		if ((rs = bwrite(ofp,buf,len)) < 0)
			break ;
#endif /* F_WRITE */

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



