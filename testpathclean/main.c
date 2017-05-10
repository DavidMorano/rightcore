/* main */

/* test the 'pathclean(3)' subroutine */


#define	CF_DEBUGS		1	/* non-switchable debug print-outs */
#define	CF_DMALLOCSHUTDOWN	0	/* already handled by 'exit(3c)' */


/******************************************************************************

	This little subroutine tests something! :-)


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<paramfile.h>
#include	<exitcodes.h>

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif

#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	shdhrink(const char *,int,char **) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* local variables */

static unsigned char	tterms[] = {
	        0x00, 0x1B, 0x00, 0x00,
	        0x01, 0x10, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00
} ;

static unsigned char	dterms[] = {
	        0x00, 0x1B, 0x00, 0x00,
	        0x01, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00
} ;


/* exported subrouines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PARAMFILE	params ;

	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	time_t	daytime ;

	int	rs, i, n ;
	int	len, sl, cl ;
	int	fd_debug = -1 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	pathbuf[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*sp, *cp, *vp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


#if	CF_DEBUGS
	debugprintf("main: program entered\n") ;
#endif

	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	bopen(ifp,BFILE_STDIN,"dr",0666) ;

	bprintf(ofp,"CLEANPATH subroutine test program\n") ;

/* loop stuff */

	while (TRUE) {

	    daytime = time(NULL) ;

	    bprintf(ofp,"path> ") ;

	    bflush(ofp) ;

	    rs = breadline(ifp,linebuf,LINEBUFLEN) ;

	    len = rs ;
	    if (rs <= 0)
	        break ;

	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;
	    sl = sfshrink(linebuf,len,&sp) ;

	    sp[sl] = '\0' ;

	bprintf(ofp,"before> %s\n",sp) ;

		rs = pathclean(pathbuf,sp,sl) ;

#if	CF_DEBUGS
	debugprintf("main: pathclean() rs=%d pathbuf=%t\n",rs,pathbuf,rs) ;
#endif

	bprintf(ofp,"after rs=%d> %s\n",rs,pathbuf) ;

		if (rs < 0)
			break ;

	} /* end while */

	bprintf(ofp,"\n") ;

	bclose(ifp) ;

	bclose(ofp) ;

#if	CF_DEBUGS
	            debugprintf("main: ret\n") ;
#endif

#if	defined(DMALLOC) && CF_DMALLOCSHUTDOWN
	debugprintf("main: dmalloc_shutdown()\n") ;
	dmalloc_shutdown() ;
#endif

	return EX_OK ;
}
/* end subroutine (main) */



