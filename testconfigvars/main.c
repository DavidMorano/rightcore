/* main */

/* test the CONFIGVARS object */


#define	CF_DEBUGS		1	/* non-switchable debug print-outs */
#define	CF_DMALLOCSHUTDOWN	0	/* already handled by 'exit(3c)' */


/******************************************************************************

	This little subroutine tests the CONFIGVARS object.


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
#include	<vecitem.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"configvars.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	CONFFNAME
#define	CONFFNAME	"conf"
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local variables */

static const unsigned char	tterms[] = {
	        0x00, 0x1B, 0x00, 0x00,
	        0x01, 0x10, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00
} ;

static const unsigned char	dterms[] = {
	        0x00, 0x1B, 0x00, 0x00,
	        0x01, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	CONFIGVARS	cvs ;

	VECITEM		errs ;

	FIELD		f ;

	time_t	daytime ;

	int	rs ;
	int	i, n ;
	int	len, sl, cl ;
	int	fd_debug = -1 ;

	const char	*sp, *cp, *vp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


#if	CF_DEBUGS
	debugprintf("main: program entered\n") ;
#endif

	bopen(ifp,BFILE_STDIN,"dr",0666) ;


	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	bprintf(ofp,"CONFIGVARS object test program\n") ;


/* initial stuff */

	vecitem_start(&errs,10,0) ;

	if ((argc > 1) && (argv[1] != NULL)) {
	    cp = argv[1] ;

	} else
	    cp = CONFFNAME ;

	    rs = configvars_open(&cvs,cp,&errs) ;

#if	CF_DEBUGS
	debugprintf("main: configvars_open() rs=%d\n",rs) ;
#endif

	{



	} /* end block */


#ifdef	COMMENT

/* loop stuff */

	while (TRUE) {

	    u_time(&daytime) ;

	    bprintf(ofp,"query name> ") ;

	    bflush(ofp) ;

	    len = breadline(ifp,linebuf,LINEBUFLEN) ;

	    if (len <= 0)
	        break ;

	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;
	    sl = sfshrink(linebuf,len,&sp) ;

	    sp[sl] = '\0' ;

/* check on the parameter file */

	    configvars_check(&cvs,daytime,&errs) ;

/* process the current query */

	    if (len > 0) {

	        PARAMFILE_CUR	cur ;


	        configvars_curbegin(&cvs,&cur) ;

	        for (i = 0 ; TRUE ; i += 1) {

	            rs = configvars_fetch(&cvs,sp,&cur,&vp) ;

#if	CF_DEBUGS
	            debugprintf("main: configvars_fetch() rs=%d\n",rs) ;
#endif

	            if (rs < 0)
	                break ;

	            bprintf(ofp,"%i value=>%s<\n",i,vp) ;

	        } /* end for */

	        configvars_curend(&cvs,&cur) ;

	    } /* end if (had a non-zero length query) */

	} /* end while */

#endif /* COMMENT */

	bprintf(ofp,"\n") ;


#if	CF_DEBUGS
	            debugprintf("main: configvars_close()\n") ;
#endif

	configvars_close(&cvs) ;

#if	CF_DEBUGS
	            debugprintf("main: vecitem_finish()\n") ;
#endif

	vecitem_finish(&errs) ;


#if	CF_DEBUGS
	            debugprintf("main: bclose(ofp)\n") ;
#endif

	bclose(ofp) ;

#if	CF_DEBUGS
	            debugprintf("main: bclose(ifp)\n") ;
#endif

	bclose(ifp) ;

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



