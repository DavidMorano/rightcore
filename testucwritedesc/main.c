/* main (testuccopy) */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_WRITE	1		/* ? */
#define	CF_LITTLE	1		/* do the little output */
#define	CF_ERRSUM	0		/* error summary */
#define	CF_READLINE	1		/* breadline() ? */


/* revision history:

	= 1988-02-01, David A­D­ Morano
        This subroutine was originally written to do some testing on the BIO
        package.

*/

/* Copyright © 1988 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a test program for the BIO package.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2

#ifndef	BUFLEN
#define	BUFLEN		200
#endif


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strdcpy1(char *,int,cchar *) ;


/* external variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	bfile	errfile, *efp ;
	bfile	outfile, *ofp = &outfile ;

	int		rs = SR_OK ;
	int		pan, i ;
	int	argl, aol ;
	int	ex = EX_INFO ;
	int	len ;
	int	lines ;
	int	fd_debug ;
	int	fd_input ;
	int	fd_output ;
	int	f_usage = FALSE ;

	cchar	*progname, *argp, *aop ;
	cchar	*cp ;
	char	buf[BUFLEN + 1], *bp ;


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

	efp = NULL ;
	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

#if	CF_DEBUGS
	debugprintf("main: about to open input\n") ;
#endif

	fd_input = FD_STDIN ;
	if ((argc >= 2) && (argv[1][0] != '\0')) {
	    rs = uc_open(argv[1],O_RDONLY,0666) ;
#if	CF_DEBUGS
	debugprintf("main: uc_open() rs=%d\n",rs) ;
#endif
		fd_input = rs ;
	}
	
	if (rs < 0)
		goto badin ;

#if	CF_DEBUGS
	debugprintf("main: about to open output\n") ;
#endif

	rs = FD_STDOUT ;
	if ((argc >= 3) && (argv[2][0] != '\0'))
	    rs = uc_open(argv[2],O_WRONLY,0666) ;

	fd_output = rs ;
	if (rs < 0)
		goto badout ;


/* OK, go into infinite loop mode */

#if	CF_DEBUGS
	debugprintf("main: before loop\n",rs) ;
#endif

	lines = 0 ;
	while (TRUE) {

	    rs = uc_copy(fd_input,fd_output,-1) ;

#if	CF_DEBUGS
	    debugprintf("main: u_copy() rs=%d\n",rs) ;
#endif

	   if (rs < 0)
		break ;

		len = rs ;
	    if (len <= 0)
	        break ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("main: after loop\n") ;
#endif

	uc_close(fd_output) ;

	uc_close(fd_input) ;

	if (len < 0) {

#if	CF_DEBUGS
	    debugprintf("main: bad read, len=%d\n",len) ;
#endif

	    goto baddone ;
	}

	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("main: bad write, rs=%d\n",rs) ;
#endif

	    goto baddone ;
	}

done:
	ex = EX_OK ;

retearly:
ret1:
	bclose(efp) ;

	return ex ;

/* usage */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [infile [outfile]]\n",
	    progname,progname) ;

	goto badret ;

/* bad stuff */
not_enough:
	ex = EX_USAGE ;
	bprintf(efp,"%s: not enough arguments supplied\n",
	    progname) ;

	goto badret ;

badin:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: could not open input (%d)\n",
	    progname,rs) ;

	goto retearly ;

badout:
	u_close(fd_input) ;

	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open output (%d)\n",
	    progname,rs) ;

	goto retearly ;

baddone:
badret:
	ex = EX_USAGE ;
	goto ret1 ;
}
/* end subroutine (main) */



