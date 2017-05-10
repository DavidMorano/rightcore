/* main */

/* program to test the terminal escape strings */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_CATCH	0


/* revision history:

	= 1988-01-01, David A­D­ Morano

	This subroutine (it's the whole program also)
	was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This program tests to see what some terminal escape strings
	do.


***************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<sys/conf.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<termstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048		/* must be greater then 1024 */
#endif

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	O_CLIFLAGS	(O_WRONLY | O_CREAT | O_NONBLOCK)
#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct termios		ots, nts ;

	struct sigaction	sigs ;

	bfile	outfile, *ofp = &outfile ;
	bfile	errfile, *efp = &errfile ;

	sigset_t		signalmask ;

	int	rs = 0 ;
	int	rs1 ;
	int	len ;
	int	i, j ;
	int	pipefds[2] ;
	int	ex = EX_INFO ;

	char	buffer[BUFLEN + 1], *bp ;
	char	*progname ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUFLEN,0) ;

	f_signal = FALSE ;
	f_alarm = FALSE ;

	bopen(&outfile,BFILE_STDOUT,"wct",0666) ;

	bprintf(&outfile,"this is before..^") ;

	bprintf(&outfile,"%s%12.0H%s\033[;10Hthis is in the middle%s\n%s%s",
		TERMSTR_SAVE,
		TERMSTR_EL, TERMSTR_EL, TERMSTR_EL,
		TERMSTR_RESTORE) ;

	bprintf(&outfile,"..this is afterwards\n") ;

	bclose(&outfile) ;

	ex = EX_OK ;

done:
ret1:
retearly:
	bclose(efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badin:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: could not open input, rs=%d\n",
	    progname,rs) ;

	goto retearly ;

badout:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open output, rs=%d\n",
	    progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


void int_alarm(sig)
int	sig ;
{

	f_alarm = TRUE ;
}


void int_signal(sig)
int	sig ;
{

	f_signal = TRUE ;
}



