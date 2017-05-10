/* main (testio) */


#define	CF_DEBUG	1
#define	F_WRITE		1
#define	F_ERROR		0


/* revision history:

	= 99/02/01, David A­D­ Morano

	This is a little (big) hack to try to test some parts
	of the BIO package.


*/


/************************************************************************

	This is a test program for the BIO package.


*************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<pwd.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define	VERSION		"1"
#define	NPARG		2

#define	BUFLEN		(8 * 1024)



/* external subroutines */


/* external variables */






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;

	int	pan, i ;
	int	argl, aol ;
	int	rs ;
	int	tlen ;
	int	sfd, dfd, err_fd ;
	int	f_usage = FALSE ;

	char	*progname, *argp, *aop ;
	char	*buf ;
	char	*cp ;


#if	F_ERROR
	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;
#endif /* F_ERROR */


#if	CF_DEBUG
	debugprint("main: entered\n") ;
		debugprintf("main: about to open error\n") ;
#endif

	progname = argv[0] ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;



	if ((rs = uc_valloc(BUFLEN,&buf)) < 0)
		return BAD ;

#if	CF_DEBUG
		debugprintf("main: about to open input\n") ;
#endif

	if ((sfd = open(argv[1],O_RDONLY,0666)) < 0) {

#if	CF_DEBUG
		debugprintf("main: no open input, rs=%d\n",rs) ;
#endif

		return BAD ;
	}


#if	CF_DEBUG
		debugprintf("main: about to open output\n") ;
#endif

	if ((dfd = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC,0666)) < 0) {

#if	CF_DEBUG
		debugprintf("main: no open output, rs=%d\n",rs) ;
#endif

		return BAD ;
	}

/* OK, go into infinite loop mode */

	tlen = uc_copy(sfd,dfd,-1) ;

#if	CF_DEBUG
		debugprintf("main: after loop\n") ;
#endif

	close(sfd) ;

	close(dfd) ;


	rs = tlen ;
	if (tlen < 0) {

#if	CF_DEBUG
		debugprintf("main: bad read, len=%d\n",tlen) ;
#endif

		goto baddone ;
	}

	if (rs < 0) {

#if	CF_DEBUG
		debugprintf("main: bad write, rs=%d\n",rs) ;
#endif

		goto baddone ;
	}


	bprintf(efp,"%s: tlen=%d\n",progname,tlen) ;


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



