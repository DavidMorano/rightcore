/* main (testuterm) */

/* program to display characters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-01-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This program provides a test for the UNIX Terminal library.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	BUFLEN	100



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	struct termios	saved ;

	long	lw ;

	int	rs ;
	int	i, j ;
	int	len, llen ;
	int	ttfd, tfd = 0 ;
	int	fd_debug ;
	int	ex = EX_INFO ;
	int	f_exit ;

	uchar	c ;
	uchar	buf[BUFLEN + 1] ;
	uchar	*bp ;

	char	*progname ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = strbasename(argv[0]) ;

#if	CF_DEBUGS
	debugprintf("main: 1\n") ;
#endif

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_NOBUF,0) ;

#if	CF_DEBUGS
	debugprintf("main: 2\n") ;
#endif

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0) {
	    ex = EX_CANTCREAT ;
	}

#if	CF_DEBUGS
	debugprintf("main: 3\n") ;
#endif


/* set up terminal for raw access */

	tfd = FD_STDIN ;
	ttfd = u_dup(tfd) ;

	uc_tcgetattr(ttfd,&saved) ;

#if	CF_DEBUGS
	debugprintf("main: about to call 'uterm_start'\n") ;
#endif

	if ((rs = uterm_start(tfd)) < 0) {

	    bprintf(efp,"%s: bad terminal initialize rs=%d\n",
	        progname,rs) ;

	    goto badret ;
	}



/* set initial terminal mode to default */

#if	CF_DEBUGS
	debugprintf("main: about to call 'uterm_control'\n") ;
#endif

	rs = uterm_control(tfd,fm_setmode,0L) ;

#if	CF_DEBUGS
	debugprintf("main: uterm_control() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    bprintf(efp,"%s: bad return from 'control' rs=%d\n",
	        progname,rs) ;


#if	CF_DEBUGS
	debugprintf("main: about to loop\n") ;
#endif

	f_exit = FALSE ;
	while (! f_exit) {

#if	CF_DEBUGS
	    debugprintf("main: about to for\n") ;
#endif

	    for (i = 0 ; i < 20 ; i += 1) {

	        bflush(ofp) ;

#if	CF_DEBUGS
	        debugprintf("main: about to 'uterm_reade'\n") ;
#endif

	        len = uterm_reade(tfd,(fm_rawin | fm_noecho),
	            (long) buf, 1L, 10000L,0L,0L, 0L) ;

#if	CF_DEBUGS
	        debugprintf("main: len read (%d)\n",len) ;
#endif

	        if (len <= 0) {

	            bprintf(efp,"\n problems w/ input, len=%d\n",len) ;

	            goto done ;
	        }

	        bprintf(ofp," %02X",buf[0]) ;

	        if ((buf[0] & 0x7F) == '\r') {

	            f_exit = TRUE ;
	            break ;

	        } else if ((buf[0] & 0x7F) == '\n')

	            bprintf(ofp,"\r\n") ;

	    } /* end for */

	    bprintf(ofp,"\n") ;

	} /* end while */


done:
	uterm_restore(tfd) ;

badret:
	uc_tcsetattr(ttfd,TCSADRAIN,&saved) ;

ret1:
	bclose(ofp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


